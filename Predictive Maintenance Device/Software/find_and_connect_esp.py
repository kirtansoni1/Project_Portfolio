import serial.tools.list_ports

ports = serial.tools.list_ports.comports()

print("🔍 Available Serial Ports:\n")
for port in ports:
    print(f"Device: {port.device}")
    print(f" - Description: {port.description}")
    print(f" - HWID: {port.hwid}")
    print()

def find_esp32_port():
    ports = serial.tools.list_ports.comports()
    for port in ports:
        hwid = port.hwid
        if "303A:1001" in hwid:
            print(f"✅ ESP32 found on {port.device}")
            return port.device
    raise RuntimeError("❌ ESP32 with VID:303A & PID:1001 not found")

try:
    port = find_esp32_port()
    print(f"Connecting to {port}...")
except Exception as e:
    print("Error:", e)