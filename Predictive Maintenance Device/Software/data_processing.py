import serial
import serial.tools.list_ports
import re
import pandas as pd
import numpy as np
import pickle
import os
from sklearn.preprocessing import StandardScaler, LabelEncoder
from datetime import datetime

# Constants for sampling rates
SAMPLES_PRESSURE = 6000   # 100 Hz
SAMPLES_FLOW = 600        # 10 Hz
SAMPLES_TEMP = 60         # 1 Hz
SAMPLES_VIB = 60          # 1 Hz
SAMPLES_VIRTUAL = 60      # 1 Hz

# Regex Patterns
pressure_pattern = re.compile(
    r'PS1:(-?\d+\.\d+)\|PS2:(-?\d+\.\d+)\|PS3:(-?\d+\.\d+)\|PS4:(-?\d+\.\d+)\|PS5:(-?\d+\.\d+)\|PS6:(-?\d+\.\d+)\|EPS1:(-?\d+\.\d+)')
flow_pattern = re.compile(r'FS1:(-?\d+\.\d+)\|FS2:(-?\d+\.\d+)')
temp_pattern = re.compile(r'TS1:(-?\d+\.\d+)\|TS2:(-?\d+\.\d+)\|TS3:(-?\d+\.\d+)\|TS4:(-?\d+\.\d+)')
vib_pattern = re.compile(r'VS1:(-?\d+\.\d+)')
virtual_pattern = re.compile(r'CP:(-?\d+\.\d+)\|CE:(-?\d+\.\d+)\|SE:(-?\d+\.\d+)')

# Loading models and preprocessing objects
MODELS_DIR = 'models'
models = {}
label_encoders = {}
scaler = None


def preprocess_data(df):
    """Apply the same preprocessing used during training"""
    # Select only the features used in training
    features = ['PS1', 'PS2', 'PS3', 'PS4', 'PS5', 'PS6', 'EPS1',
                'FS1', 'FS2', 'TS1', 'TS2', 'TS3', 'TS4',
                'VS1', 'SE', 'CE', 'CP']
    X = df[features]

    # Apply scaling
    X_scaled = scaler.transform(X)
    return X_scaled


def make_predictions(df):
    """Make predictions for all targets"""
    # Preprocess the data
    X = preprocess_data(df)

    predictions = {}
    for target, model in models.items():
        pred_encoded = model.predict(X)
        predictions[target] = label_encoders[target].inverse_transform(pred_encoded)

    return predictions

try:
    # Loading scaler
    with open(f'{MODELS_DIR}/scaler.pkl', 'rb') as f:
        scaler = pickle.load(f)

    # Load models and label encoders
    for target in ['cool_cond', 'valve_cond', 'pump_leak', 'hyd_accum', 'stable_flag']:
        with open(f'{MODELS_DIR}/best_{target}.pkl', 'rb') as f:
            models[target] = pickle.load(f)
        with open(f'{MODELS_DIR}/label_encoder_{target}.pkl', 'rb') as f:
            label_encoders[target] = pickle.load(f)
except Exception as e:
    print(f"Error loading models: {e}")
    exit()

# Detect ESP32 COM Port
def find_esp32_port():
    ports = serial.tools.list_ports.comports()
    for port in ports:
        if "303A:1001" in port.hwid:
            print(f"ESP32 found on {port.device}")
            return port.device
    raise RuntimeError("ESP32 not found")

# Initialize serial
try:
    port = find_esp32_port()
    ser = serial.Serial(port, 250000, timeout=2)
except Exception as e:
    print("Error opening serial port:", e)
    exit()

print("Listening for data...")

# Buffers
pressure_rows = []
flow_rows = [None] * SAMPLES_PRESSURE
temp_rows = [None] * SAMPLES_PRESSURE
vib_rows = [None] * SAMPLES_PRESSURE
virtual_rows = [None] * SAMPLES_PRESSURE

pressure_idx = 0
flow_idx = 0
temp_idx = 0
vib_idx = 0
virt_idx = 0
buffering = False

while True:
    try:
        line = ser.readline().decode('utf-8').strip()

        if line == "START":
            print("Started receiving sensor data")
            pressure_rows.clear()
            flow_rows = [None] * SAMPLES_PRESSURE
            temp_rows = [None] * SAMPLES_PRESSURE
            vib_rows = [None] * SAMPLES_PRESSURE
            virtual_rows = [None] * SAMPLES_PRESSURE
            pressure_idx = flow_idx = temp_idx = vib_idx = virt_idx = 0
            buffering = True
            continue

        elif line == "END":
            print("Transmission complete. Creating DataFrame...")

            # Create DataFrame
            df = pd.DataFrame(pressure_rows, columns=[
                "PS1", "PS2", "PS3", "PS4", "PS5", "PS6", "EPS1"
            ])

            for i in range(SAMPLES_FLOW):
                idx = i * 10
                if idx < len(df) and flow_rows[idx]:
                    df.loc[idx, ["FS1", "FS2"]] = flow_rows[idx]

            for i in range(SAMPLES_TEMP):
                idx = i * 100
                if idx < len(df) and temp_rows[idx]:
                    df.loc[idx, ["TS1", "TS2", "TS3", "TS4"]] = temp_rows[idx]

            for i in range(SAMPLES_VIB):
                idx = i * 100
                if idx < len(df) and vib_rows[idx] is not None:
                    df.loc[idx, "VS1"] = vib_rows[idx]

            for i in range(SAMPLES_VIRTUAL):
                idx = i * 100
                if idx < len(df) and virtual_rows[idx]:
                    df.loc[idx, ["CP", "CE", "SE"]] = virtual_rows[idx]

            # Forward fill missing values
            df.fillna(method='ffill', inplace=True)

            # Save raw data first
            df.to_csv("final_sensor_data.csv", index=False)
            print("Raw data saved to final_sensor_data.csv")

            #predictions
            try:
                predictions = make_predictions(df)

                # Add predictions to DataFrame
                for target, preds in predictions.items():
                    df[f'pred_{target}'] = preds

                # Save data with predictions
                df.to_csv("final_sensor_data_with_predictions.csv", index=False)
                print("Data with predictions saved to final_sensor_data_with_predictions.csv")

                # Printing some prediction stats
                for target in predictions.keys():
                    unique, counts = np.unique(predictions[target], return_counts=True)
                    print(f"{target}: {dict(zip(unique, counts))}")

            except Exception as e:
                print(f"Prediction error: {e}")

            buffering = False
            continue

        elif line=="USB OFF!":
            print("USB is OFF, Press USB_EN button to start data transfer.")
            buffering = False

        if buffering:
            if m := pressure_pattern.match(line):
                if pressure_idx < SAMPLES_PRESSURE:
                    pressure_rows.append([float(m.group(i)) for i in range(1, 8)])
                    pressure_idx += 1

            elif m := flow_pattern.match(line):
                if flow_idx < SAMPLES_FLOW:
                    flow_rows[flow_idx * 10] = [float(m.group(1)), float(m.group(2))]
                    flow_idx += 1

            elif m := temp_pattern.match(line):
                if temp_idx < SAMPLES_TEMP:
                    temp_rows[temp_idx * 100] = [float(m.group(i)) for i in range(1, 5)]
                    temp_idx += 1

            elif m := vib_pattern.match(line):
                if vib_idx < SAMPLES_VIB:
                    vib_rows[vib_idx * 100] = float(m.group(1))
                    vib_idx += 1

            elif m := virtual_pattern.match(line):
                if virt_idx < SAMPLES_VIRTUAL:
                    virtual_rows[virt_idx * 100] = [float(m.group(i)) for i in range(1, 4)]
                    virt_idx += 1

    except KeyboardInterrupt:
        print("Stopped by user.")
        break
    except Exception as e:
        print("Error:", e)
