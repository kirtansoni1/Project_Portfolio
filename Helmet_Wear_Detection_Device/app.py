import os
import sqlite3
from datetime import datetime, timedelta, timezone
from threading import Thread
from flask import Flask, request, jsonify, render_template, g, abort

APP_DB = os.path.join(os.path.dirname(__file__), "helmet.db")

app = Flask(__name__)

# ----------------------
# DB utilities
# ----------------------
def get_db():
    db = getattr(g, "_db", None)
    if db is None:
        db = g._db = sqlite3.connect(APP_DB, detect_types=sqlite3.PARSE_DECLTYPES)
        db.row_factory = sqlite3.Row
    return db

@app.teardown_appcontext
def close_db(_exc):
    db = getattr(g, "_db", None)
    if db is not None:
        db.close()

def init_db():
    db = get_db()
    db.execute("""
        CREATE TABLE IF NOT EXISTS events (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            state TEXT NOT NULL CHECK (state IN ('ON','OFF')),
            ts_utc TEXT NOT NULL
        )
    """)
    db.execute("CREATE INDEX IF NOT EXISTS idx_events_ts ON events(ts_utc)")
    db.commit()

def last_event():
    db = get_db()
    row = db.execute("SELECT state, ts_utc FROM events ORDER BY ts_utc DESC LIMIT 1").fetchone()
    return dict(row) if row else None

def insert_event(state: str, ts: datetime):
    db = get_db()
    db.execute("INSERT INTO events(state, ts_utc) VALUES (?, ?)", (state, ts.isoformat().replace("+00:00","Z")))
    db.commit()

def last_event_before(ts: datetime):
    db = get_db()
    row = db.execute(
        "SELECT state, ts_utc FROM events WHERE ts_utc < ? ORDER BY ts_utc DESC LIMIT 1",
        (ts.isoformat().replace("+00:00","Z"),),
    ).fetchone()
    return dict(row) if row else None

def events_between(start: datetime, end: datetime):
    db = get_db()
    rows = db.execute(
        "SELECT state, ts_utc FROM events WHERE ts_utc >= ? AND ts_utc <= ? ORDER BY ts_utc ASC",
        (start.isoformat().replace("+00:00","Z"), end.isoformat().replace("+00:00","Z")),
    ).fetchall()
    return [dict(r) for r in rows]

# ----------------------
# Helpers
# ----------------------
def utcnow():
    return datetime.now(timezone.utc)

def to_bool(state: str) -> int:
    return 1 if state.upper() == "ON" else 0

# ----------------------
# Routes
# ----------------------
@app.route("/")
def index():
    return render_template("index.html")

@app.route("/state", methods=["POST"])
def post_state():
    """
    Payload: {"state":"ON"} or {"state":"OFF"}
    - Dedupes if the last state is same (no-op, 200).
    """
    try:
        data = request.get_json(force=True, silent=False)
    except Exception:
        abort(400, description="Invalid JSON")

    if not data or "state" not in data:
        abort(400, description="Expected payload: {'state':'ON'|'OFF'}")

    state = str(data["state"]).strip().upper()
    if state not in ("ON", "OFF"):
        abort(400, description="state must be 'ON' or 'OFF'")

    now = utcnow()
    last = last_event()
    if last and last["state"] == state:
        # idempotent; avoid DB bloat when firmware misbehaves
        return jsonify({"ok": True, "dedup": True, "state": state})

    insert_event(state, now)
    return jsonify({"ok": True, "state": state, "ts_utc": now.isoformat().replace("+00:00","Z")})

@app.route("/api/status")
def api_status():
    """
    Current status and last change timestamp.
    """
    last = last_event()
    if not last:
        return jsonify({"state": "OFF", "last_change_utc": None})
    return jsonify({"state": last["state"], "last_change_utc": last["ts_utc"]})

@app.route("/api/logs")
def api_logs():
    """
    Returns raw event list for the last N hours (default 24).
    """
    hours = int(request.args.get("hours", 24))
    end = utcnow()
    start = end - timedelta(hours=hours)
    evs = events_between(start, end)
    return jsonify({"start_utc": start.isoformat().replace("+00:00","Z"),
                    "end_utc": end.isoformat().replace("+00:00","Z"),
                    "events": evs})

@app.route("/api/sessions")
def api_sessions():
    """
    Coalesces ON periods into sessions with durations for last N hours.
    """
    hours = int(request.args.get("hours", 24))
    end = utcnow()
    start = end - timedelta(hours=hours)

    # Build a state stream starting from start boundary
    prior = last_event_before(start)
    current_state = prior["state"] if prior else "OFF"
    stream = [{"state": current_state, "ts_utc": start.isoformat().replace("+00:00","Z")}]
    stream.extend(events_between(start, end))
    stream.append({"state": current_state, "ts_utc": end.isoformat().replace("+00:00","Z")})  # will update below

    # Recompute tail state to 'last event'
    if len(stream) >= 2:
        stream[-1]["state"] = last_event()["state"] if last_event() else current_state

    # Build sessions of ON
    sessions = []
    on_start = None
    prev_state = stream[0]["state"]
    prev_ts = datetime.fromisoformat(stream[0]["ts_utc"].replace("Z","+00:00"))

    for i in range(1, len(stream)):
        st = stream[i]["state"]
        ts = datetime.fromisoformat(stream[i]["ts_utc"].replace("Z","+00:00"))

        if prev_state == "ON":
            # An ON segment from prev_ts until ts
            if on_start is None:
                on_start = prev_ts
        else:
            # OFF -> do nothing
            pass

        # State change boundary
        if st != prev_state:
            # Inside api_sessions, when state changes from ON to OFF:
            if prev_state == "ON":
                sessions.append({
                    "start_utc": on_start.isoformat().replace("+00:00","Z"),
                    "end_utc": ts.isoformat().replace("+00:00","Z"),   # use current OFF ts
                    "duration_sec": int((ts - on_start).total_seconds())
                })
                on_start = None

        prev_state, prev_ts = st, ts

    # If still ON at end, record running session till 'end'
    if prev_state == "ON" and on_start:
        sessions.append({
            "start_utc": on_start.isoformat().replace("+00:00","Z"),
            "end_utc": None,
            "duration_sec": int((end - on_start).total_seconds())
        })

    return jsonify({"sessions": sessions})

@app.route("/api/series")
def api_series():
    hours = int(request.args.get("hours", 24))
    end = utcnow()
    start = end - timedelta(hours=hours)

    prior = last_event_before(start)
    cur_val = to_bool(prior["state"]) if prior else 0

    points = []
    points.append({"x": start.isoformat().replace("+00:00","Z"), "y": cur_val})

    for ev in events_between(start, end):
        ev_ts = ev["ts_utc"]
        ev_val = to_bool(ev["state"])
        # step shape: flat until change
        points.append({"x": ev_ts, "y": cur_val})
        cur_val = ev_val
        points.append({"x": ev_ts, "y": cur_val})

    points.append({"x": end.isoformat().replace("+00:00","Z"), "y": cur_val})
    return jsonify({"points": points})

@app.route("/api/health")
def api_health():
    return jsonify({"ok": True})

# ----------------------
# Entrypoint
# ----------------------
if __name__ == "__main__":
    with app.app_context():
        init_db()
    # listen on LAN so ESP32 can reach us
    app.run(host="0.0.0.0", port=5000, debug=False, threaded=True)
