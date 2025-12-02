/**
  * Helmet_wear_detection_device
  * This Device detects whether a helmet is being worn or not.
  * It connects to a Wi-Fi network to send status updates.
  */
#include "main.h"
#include <stdarg.h>

/* =========================
 *  GLOBALS
 * ========================= */
Adafruit_VL6180X g_vl6180x;
static DeviceState g_lastDecision = STATE_OFF;      // last measured/classified
static int8_t      g_lastReported = -1;             // last sent to server (-1 = unknown)
static uint32_t    g_lastPostMs   = 0;
static uint32_t    g_bootMs = 0;

/* =========================
 *  LOGGING
 * ========================= */
static void logE(const char* fmt, ...) {
  va_list args; va_start(args, fmt);
  char buf[200]; vsnprintf(buf, sizeof(buf), fmt, args); va_end(args);
  Serial.print("[ERR] "); Serial.println(buf);
}
static void logI(const char* fmt, ...) {
#if LOG_ERRORS_ONLY == 0
  va_list args; va_start(args, fmt);
  char buf[200]; vsnprintf(buf, sizeof(buf), fmt, args); va_end(args);
  Serial.print("[INF] "); Serial.println(buf);
#endif
}

/* =========================
 *  POWER
 * ========================= */

static inline bool usbAttached() {
  // Grace period after boot (lets esptool / Arduino IDE attach reliably)
  if (millis() - g_bootMs < DEV_NO_SLEEP_BOOT_MS) return true;

  // If Serial is active, assume developer is connected
  if (Serial) return true;

  return false;
}

// Light-sleep for ~ms; disables Wi-Fi modem beforehand to save power.
// Skips sleeping entirely if USB is attached (keeps CDC stable for flashing/monitoring).
static void lightSleepMs(uint32_t ms) {
  if (ms == 0) return;

  if (usbAttached()) {
    Serial.println("[WARN] USB Connected skipping sleep");
    delay(ms); // stay fully awake while tethered
    return;
  }

  // Make sure Wi-Fi is fully off while idling
  if (WiFi.getMode() != WIFI_OFF && WiFi.status() != WL_CONNECTED) {
    // If not connected, shut radio down to save power
    WiFi.disconnect(true /*wifioff*/);
    WiFi.mode(WIFI_OFF);
  }

  // Configure timer wakeup
  uint64_t us = (uint64_t)(ms + SLEEP_TIMER_SAFETY_MS) * 1000ULL;
  esp_sleep_enable_timer_wakeup(us);

  // Enter light sleep (RAM/regs retained; wakes on timer)
  esp_light_sleep_start();

  delay(1);
}

/* =========================
 *  SENSOR
 * ========================= */
static bool readOneRangeMm(uint16_t& outMm) {
  uint8_t r = g_vl6180x.readRange();
  uint8_t s = g_vl6180x.readRangeStatus();   // 0=valid, 7=no target
  if (s == 0 || s == 7) { outMm = r; return true; }
  return false;
}

static DeviceState classify(DeviceState lastKnown) {
  uint8_t onHits = 0, offHits = 0;
  for (uint8_t i = 0; i < DEBOUNCE_COUNT; ++i) {
    uint16_t mm = 0xFFFF;
    if (readOneRangeMm(mm)) {
      if (mm <= THRESH_MM) onHits++;
      else if (mm >= (THRESH_MM + HYST_MM)) offHits++;
    }
    if (onHits  >= DEBOUNCE_COUNT) { logI("Helmet Worn");    return STATE_ON;  }
    if (offHits >= DEBOUNCE_COUNT) { logI("Helmet Removed"); return STATE_OFF; }
    delay(SAMPLE_GAP_MS);
  }
  if (onHits > offHits)  return STATE_ON;
  if (offHits > onHits)  return STATE_OFF;
  return lastKnown;
}

/* =========================
 *  NET
 * ========================= */
static bool wifiOnAndConnect() {
#if ENABLE_NET
  if (WiFi.status() == WL_CONNECTED){
    logI("WiFi auto connected, nice!");
    return true;
  }
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - t0) < WIFI_CONNECT_BUDGET_MS) {
    delay(100);
  }
  if (WiFi.status() != WL_CONNECTED) {
    logE("WiFi connect timeout, status=%d", (int)WiFi.status());
    return false;
  }
  logI("WiFi OK: %s", WiFi.localIP().toString().c_str());
  return true;
#else
  return false;
#endif
}

// return true only if a POST succeeded
static bool maybePost(DeviceState s) {
#if ENABLE_NET
  if (!wifiOnAndConnect()) return false;

  const char* lit = (s == STATE_ON) ? "ON" : "OFF";
  String payload = String("{\"state\":\"") + lit + "\"}";

  for (uint8_t tries = 0; tries < REPORT_TRIES; ++tries) {
    HTTPClient http;                   
    http.setTimeout(HTTP_TIMEOUT_MS);
    if (!http.begin(FLASK_URL)) {
      logE("HTTP begin failed");
      continue;
    }
    http.addHeader("Content-Type", "application/json");
    int code = http.POST(payload);
    http.end();

    if (code >= 200 && code < 300) {
      logI("POST %s -> %d", lit, code);

      // Hard power-down the radio to save battery after a successful post
      WiFi.disconnect(true /*wifioff*/);
      WiFi.mode(WIFI_OFF);
      return true;
    } else {
      logE("POST %s -> %d", lit, code);
    }
    delay(REPORT_RETRY_GAP_MS);
  }
  WiFi.disconnect(true);
  WiFi.mode(WIFI_OFF);
  return false;
#else
  (void)s;
  return false;
#endif
}


/* =========================
 *  LIFECYCLE
 * ========================= */
void setup() {
  g_bootMs = millis();  // capture boot start time
  Serial.begin(115200);
  delay(60);

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

  if (!g_vl6180x.begin()) {
    logE("[VL6180X] init failed (check wiring & 3V3).");
    while (true) { delay(1000); }
  }
  logI("[VL6180X] init OK");

#if POST_ON_BOOT
  // Force one announcement on boot
  DeviceState bootState = classify(STATE_OFF);
  g_lastDecision = bootState;
  maybePost(bootState);
  g_lastReported = (int8_t)bootState;
  g_lastPostMs = millis();
#endif
}

void loop() {
  DeviceState s = classify(g_lastDecision);
  g_lastDecision = s;

  bool dueHeartbeat = (HEARTBEAT_SEC > 0) &&
                      (millis() - g_lastPostMs >= (uint32_t)HEARTBEAT_SEC * 1000UL);
  bool changed = ((int8_t)s != g_lastReported);

  if (changed || dueHeartbeat) {
    bool ok = maybePost(s);
    if (ok) {                      // <-- only mark reported on success
      g_lastReported = (int8_t)s;
      g_lastPostMs   = millis();
    }
  }

  lightSleepMs(POLL_PERIOD_SEC * 1000UL);
}