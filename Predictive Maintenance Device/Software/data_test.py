# === Python Receiver for Binary Protocol with CRC32 ===
import struct
import serial
import serial.tools.list_ports
import zlib
import pandas as pd
import numpy as np
import pickle
from sklearn.preprocessing import StandardScaler, LabelEncoder

RECORD_SIZE = 72  # 17 floats (68 bytes) + 4-byte CRC32
NUM_RECORDS = 6000

record_struct = struct.Struct('<17fI')  # 17 floats + CRC32

# Load models and preprocessing
MODELS_DIR = 'models'
models = {}
label_encoders = {}
scaler = None

with open(f'{MODELS_DIR}/scaler.pkl', 'rb') as f:
    scaler = pickle.load(f)

for target in ['cool_cond', 'valve_cond', 'pump_leak', 'hyd_accum', 'stable_flag']:
    with open(f'{MODELS_DIR}/best_{target}.pkl', 'rb') as f:
        models[target] = pickle.load(f)
    with open(f'{MODELS_DIR}/label_encoder_{target}.pkl', 'rb') as f:
        label_encoders[target] = pickle.load(f)

def preprocess_data(df):
    features = ['PS1', 'PS2', 'PS3', 'PS4', 'PS5', 'PS6', 'EPS1',
                'FS1', 'FS2', 'TS1', 'TS2', 'TS3', 'TS4',
                'VS1', 'CP', 'CE', 'SE']
    X = df[features]
    return scaler.transform(X)

def make_predictions(df):
    X = preprocess_data(df)
    predictions = {}
    for target, model in models.items():
        pred_encoded = model.predict(X)
        predictions[target] = label_encoders[target].inverse_transform(pred_encoded)
    return predictions

def find_esp32_port():
    ports = serial.tools.list_ports.comports()
    for port in ports:
        if "303A:1001" in port.hwid:
            print(f"ESP32 found on {port.device}")
            return port.device
    raise RuntimeError("ESP32 not found")

def read_exactly(ser, n):
    buf = bytearray()
    while len(buf) < n:
        chunk = ser.read(n - len(buf))
        if not chunk:
            raise TimeoutError("Serial read timed out")
        buf.extend(chunk)
    return buf

def read_all_records(ser):
    print("Receiving binary data...")
    records = []
    for i in range(NUM_RECORDS):
        data = read_exactly(ser, RECORD_SIZE)
        *floats, received_crc = record_struct.unpack(data)
        calc_crc = zlib.crc32(data[:-4]) & 0xFFFFFFFF
        if calc_crc != received_crc:
            print(f"CRC mismatch at index {i}")
            continue
        records.append(floats)
    return records

def create_dataframe(records):
    df = pd.DataFrame(records, columns=[
        "PS1", "PS2", "PS3", "PS4", "PS5", "PS6", "EPS1",
        "FS1", "FS2", "TS1", "TS2", "TS3", "TS4",
        "VS1", "CP", "CE", "SE"])
    return df

# Main logic
port = find_esp32_port()
ser = serial.Serial(port, 250000, timeout=5)
records = read_all_records(ser)
df = create_dataframe(records)
df.to_csv("final_sensor_data.csv", index=False)
print("Raw data saved to final_sensor_data.csv")

# try:
#     predictions = make_predictions(df)
#     for target, preds in predictions.items():
#         df[f'pred_{target}'] = preds
#     df.to_csv("final_sensor_data_with_predictions.csv", index=False)
#     print("Data with predictions saved to final_sensor_data_with_predictions.csv")
#     for target in predictions.keys():
#         unique, counts = np.unique(predictions[target], return_counts=True)
#         print(f"{target}: {dict(zip(unique, counts))}")
# except Exception as e:
#     print(f"Prediction error: {e}")
