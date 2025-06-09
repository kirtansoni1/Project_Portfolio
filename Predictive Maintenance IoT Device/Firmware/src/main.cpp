#include "main.h"

// Sensor Objects
Adafruit_BMP280 bmp1(BMP_CS_PINS[0], MOSI_PIN, MISO_PIN, SCK_PIN);
Adafruit_BMP280 bmp2(BMP_CS_PINS[1], MOSI_PIN, MISO_PIN, SCK_PIN);
Adafruit_BMP280 bmp3(BMP_CS_PINS[2], MOSI_PIN, MISO_PIN, SCK_PIN);
Adafruit_BMP280 bmp4(BMP_CS_PINS[3], MOSI_PIN, MISO_PIN, SCK_PIN);
Adafruit_BMP280 bmp5(BMP_CS_PINS[4], MOSI_PIN, MISO_PIN, SCK_PIN);
Adafruit_BMP280 bmp6(BMP_CS_PINS[5], MOSI_PIN, MISO_PIN, SCK_PIN);
Adafruit_ADXL345_Unified accel;

// Flow Sensor Variables
volatile uint32_t flow_frequency_1 = 0, flow_frequency_2 = 0;

// Buffers for Storing Sensor Data
float pressure_buffer[SAMPLES_PRESSURE][NUM_OF_PRESSURE_SENSORS];
float voltage_buffer[SAMPLES_VOLTAGE];
float current_buffer[SAMPLES_CURRENT];
float power_buffer[SAMPLES_POWER];
float flow_buffer[SAMPLES_FLOW][NUM_OF_FLOW_SENSORS];
float temperature_buffer[SAMPLES_TEMPERATURE][NUM_OF_TEMP_SENSORS];
float vibration_buffer[SAMPLES_VIBRATION];
float ce_buffer[SAMPLES_CE];
float cp_buffer[SAMPLES_CP];
float se_buffer[SAMPLES_SE];

#pragma pack(push, 1)
typedef struct {
    float ps[6];     // PS1–PS6
    float eps1;
    float fs1, fs2;
    float ts1, ts2, ts3, ts4;
    float vs1;
    float cp, ce, se;
    uint32_t crc32;
} SensorRecord;
#pragma pack(pop)

int sample_count = 0;
SemaphoreHandle_t sampleMutex;
portMUX_TYPE flowMux = portMUX_INITIALIZER_UNLOCKED;
bool usb_enable_flag = true;

void setup()
{
    Serial.begin(BAUD_RATE);

    // initializeSensors();
    sampleMutex = xSemaphoreCreateMutex();

    pinMode(USB_EN_BUTTN_PIN, INPUT);
    pinMode(USB_EN_LED_PIN, OUTPUT);
    attachInterrupt(digitalPinToInterrupt(USB_EN_BUTTN_PIN), usb_en, FALLING);

    pinMode(FLOW_SENSOR_1, INPUT_PULLUP);
    pinMode(FLOW_SENSOR_2, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_1), flow1, RISING);
    attachInterrupt(digitalPinToInterrupt(FLOW_SENSOR_2), flow2, RISING);

    Creating FreeRTOS Tasks (Pinned to Cores for Performance)
    xTaskCreatePinnedToCore(readPressureSensors, "PressureTask", 8192, NULL, 3, NULL, 0);
    xTaskCreatePinnedToCore(readVoltageCurrentSensors, "VoltageCurrentTask", 8192, NULL, 3, NULL, 0);
    xTaskCreatePinnedToCore(readFlowSensors, "FlowTask", 4096, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(readTemperatureSensors, "TemperatureTask", 4096, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(readVibrationSensor, "VibrationTask", 4096, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(incrementSampleCount, "SampleCounter", 8192, NULL, 2, NULL, 1);
}

void initializeSensors()
{
    if (!bmp1.begin())
    {
        Serial.println("BMP280 #1 not found!");
    }
    if (!bmp2.begin())
    {
        Serial.println("BMP280 #2 not found!");
    }
    if (!bmp3.begin())
    {
        Serial.println("BMP280 #3 not found!");
    }
    if (!bmp4.begin())
    {
        Serial.println("BMP280 #4 not found!");
    }
    if (!bmp5.begin())
    {
        Serial.println("BMP280 #5 not found!");
    }
    if (!bmp6.begin())
    {
        Serial.println("BMP280 #6 not found!");
    }

    /* Custom settings to increase the data rate. */
    bmp1.setSampling(Adafruit_BMP280::MODE_NORMAL,   /* Operating Mode. */
                     Adafruit_BMP280::SAMPLING_X2,   /* Temp. oversampling */
                     Adafruit_BMP280::SAMPLING_X1,   /* Pressure oversampling */
                     Adafruit_BMP280::FILTER_OFF,    /* Filtering. */
                     Adafruit_BMP280::STANDBY_MS_1); /* Standby time. */

    bmp2.setSampling(Adafruit_BMP280::MODE_NORMAL,   /* Operating Mode. */
                     Adafruit_BMP280::SAMPLING_X2,   /* Temp. oversampling */
                     Adafruit_BMP280::SAMPLING_X1,   /* Pressure oversampling */
                     Adafruit_BMP280::FILTER_OFF,    /* Filtering. */
                     Adafruit_BMP280::STANDBY_MS_1); /* Standby time. */

    bmp3.setSampling(Adafruit_BMP280::MODE_NORMAL,   /* Operating Mode. */
                     Adafruit_BMP280::SAMPLING_X2,   /* Temp. oversampling */
                     Adafruit_BMP280::SAMPLING_X1,   /* Pressure oversampling */
                     Adafruit_BMP280::FILTER_OFF,    /* Filtering. */
                     Adafruit_BMP280::STANDBY_MS_1); /* Standby time. */

    bmp4.setSampling(Adafruit_BMP280::MODE_NORMAL,   /* Operating Mode. */
                     Adafruit_BMP280::SAMPLING_X2,   /* Temp. oversampling */
                     Adafruit_BMP280::SAMPLING_X1,   /* Pressure oversampling */
                     Adafruit_BMP280::FILTER_OFF,    /* Filtering. */
                     Adafruit_BMP280::STANDBY_MS_1); /* Standby time. */

    bmp5.setSampling(Adafruit_BMP280::MODE_NORMAL,   /* Operating Mode. */
                     Adafruit_BMP280::SAMPLING_X2,   /* Temp. oversampling */
                     Adafruit_BMP280::SAMPLING_X1,   /* Pressure oversampling */
                     Adafruit_BMP280::FILTER_OFF,    /* Filtering. */
                     Adafruit_BMP280::STANDBY_MS_1); /* Standby time. */

    bmp6.setSampling(Adafruit_BMP280::MODE_NORMAL,   /* Operating Mode. */
                     Adafruit_BMP280::SAMPLING_X2,   /* Temp. oversampling */
                     Adafruit_BMP280::SAMPLING_X1,   /* Pressure oversampling */
                     Adafruit_BMP280::FILTER_OFF,    /* Filtering. */
                     Adafruit_BMP280::STANDBY_MS_1); /* Standby time. */

    if (!accel.begin())
    {
        Serial.println("ADXL345 not found!");
    }
    accel.setRange(ADXL345_RANGE_16_G);
    accel.setDataRate(ADXL345_DATARATE_200_HZ);
}

// === Custom CRC32 Implementation ===
uint32_t crc32(const uint8_t *data, size_t length, uint32_t previous_crc = 0xFFFFFFFF)
{
    uint32_t crc = previous_crc;
    while (length--)
    {
        crc ^= *data++;
        for (uint8_t k = 0; k < 8; k++)
            crc = (crc >> 1) ^ (0xEDB88320UL & -(crc & 1));
    }
    return ~crc;
}

void logTransmissionTime(uint32_t start_ms, uint32_t end_ms)
{
    uint32_t duration = end_ms - start_ms;
    Serial.printf("USB transmission duration: %lu ms\n", duration);
}

void transmitSensorDataBinary()
{
    uint32_t start_time = millis();

    SensorRecord record;
    for (int i = 0; i < SAMPLES_PRESSURE; ++i)
    {
        // Pressure + EPS1
        memcpy(record.ps, pressure_buffer[i], sizeof(float) * 6);
        record.eps1 = power_buffer[i];

        // Flow (index scaled)
        int flow_idx = i / 10;
        record.fs1 = (flow_idx < SAMPLES_FLOW) ? flow_buffer[flow_idx][0] : 0;
        record.fs2 = (flow_idx < SAMPLES_FLOW) ? flow_buffer[flow_idx][1] : 0;

        // Temperature (index scaled)
        int temp_idx = i / 100;
        if (temp_idx < SAMPLES_TEMPERATURE) {
            memcpy(&record.ts1, temperature_buffer[temp_idx], sizeof(float) * 4);
        } else {
            record.ts1 = record.ts2 = record.ts3 = record.ts4 = 0;
        }

        // Vibration
        record.vs1 = (temp_idx < SAMPLES_VIBRATION) ? vibration_buffer[temp_idx] : 0;

        // Virtual metrics
        record.cp = (temp_idx < SAMPLES_CP) ? cp_buffer[temp_idx] : 0;
        record.ce = (temp_idx < SAMPLES_CE) ? ce_buffer[temp_idx] : 0;
        record.se = (temp_idx < SAMPLES_SE) ? se_buffer[temp_idx] : 0;

        // Compute CRC
        record.crc32 = crc32((uint8_t*)&record, sizeof(SensorRecord) - 4);

        // Send
        Serial.write((uint8_t*)&record, sizeof(SensorRecord));
    }

    uint32_t end_time = millis();
    Serial.printf("Binary transmission done in %lu ms\n", end_time - start_time);
}

void transmitSensorData()
{
    const size_t TX_BUFFER_SIZE = 4096;
    char tx_buffer[TX_BUFFER_SIZE];
    int offset = 0;

    Serial.println("START");

    uint32_t start_time = millis();  // <-- Start timing

    if (!xSemaphoreTake(sampleMutex, portMAX_DELAY)) return;

    // --- Pressure + Power (6000 lines expected) ---
    for (int i = 0; i < SAMPLES_PRESSURE; i++)
    {
        int len = snprintf(tx_buffer + offset, TX_BUFFER_SIZE - offset,
            "PS1:%0.3f|PS2:%0.3f|PS3:%0.3f|PS4:%0.3f|PS5:%0.3f|PS6:%0.3f|EPS1:%0.3f\n",
            pressure_buffer[i][0], pressure_buffer[i][1], pressure_buffer[i][2],
            pressure_buffer[i][3], pressure_buffer[i][4], pressure_buffer[i][5],
            power_buffer[i]);

        offset += len;
        if (offset > TX_BUFFER_SIZE - 128) {
            Serial.write((uint8_t*)tx_buffer, offset);
            offset = 0;
        }
    }

    // --- Flow ---
    for (int i = 0; i < SAMPLES_FLOW; i++)
    {
        int len = snprintf(tx_buffer + offset, TX_BUFFER_SIZE - offset,
            "FS1:%0.3f|FS2:%0.3f\n", flow_buffer[i][0], flow_buffer[i][1]);

        offset += len;
        if (offset > TX_BUFFER_SIZE - 64) {
            Serial.write((uint8_t*)tx_buffer, offset);
            offset = 0;
        }
    }

    // --- Temperature + Virtual ---
    for (int i = 0; i < SAMPLES_TEMPERATURE; i++)
    {
        int len1 = snprintf(tx_buffer + offset, TX_BUFFER_SIZE - offset,
            "TS1:%0.2f|TS2:%0.2f|TS3:%0.2f|TS4:%0.2f\n",
            temperature_buffer[i][0], temperature_buffer[i][1],
            temperature_buffer[i][2], temperature_buffer[i][3]);

        offset += len1;
        if (offset > TX_BUFFER_SIZE - 64) {
            Serial.write((uint8_t*)tx_buffer, offset);
            offset = 0;
        }

        int len2 = snprintf(tx_buffer + offset, TX_BUFFER_SIZE - offset,
            "CE:%0.2f|CP:%0.2f|SE:%0.2f\n",
            ce_buffer[i], cp_buffer[i], se_buffer[i]);

        offset += len2;
        if (offset > TX_BUFFER_SIZE - 64) {
            Serial.write((uint8_t*)tx_buffer, offset);
            offset = 0;
        }
    }

    // --- Vibration ---
    for (int i = 0; i < SAMPLES_VIBRATION; i++)
    {
        int len = snprintf(tx_buffer + offset, TX_BUFFER_SIZE - offset,
            "VS1:%0.3f\n", vibration_buffer[i]);

        offset += len;
        if (offset > TX_BUFFER_SIZE - 64) {
            Serial.write((uint8_t*)tx_buffer, offset);
            offset = 0;
        }
    }

    if (offset > 0) {
        Serial.write((uint8_t*)tx_buffer, offset);
    }

    xSemaphoreGive(sampleMutex);

    Serial.println("END");

    uint32_t end_time = millis();  // <-- End timing
    logTransmissionTime(start_time, end_time);  // Log it
}



void incrementSampleCount(void *parameter)
{
    TickType_t lastWakeTime = xTaskGetTickCount();
    for (;;)
    {
        if (xSemaphoreTake(sampleMutex, portMAX_DELAY))
        {
            // Control USB EN LED
            if (usb_enable_flag){
                digitalWrite(USB_EN_LED_PIN, HIGH);
            }
            else{
                digitalWrite(USB_EN_LED_PIN, LOW);
            }

            sample_count++;
            if (sample_count >= SAMPLES_PRESSURE)
            {

                xSemaphoreGive(sampleMutex);  // Release first

                if (usb_enable_flag)
                {
                    // transmitSensorData(); // Call data transmitter safely
                    transmitSensorDataBinary();
                }
                else
                {
                    Serial.println("USB OFF!");
                }
                sample_count = 0;            // Reset index
                continue;                    // Skip second xSemaphoreGive()
            }
            xSemaphoreGive(sampleMutex);
        }
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(10)); // 100Hz
    }
}

void readVoltageCurrentSensors(void *parameter)
{
    TickType_t lastWakeTime = xTaskGetTickCount();
    for (;;)
    {
        float voltageSum = 0, currentSum = 0;
        for (int i = 0; i < 5; i++)
        {
            voltageSum += analogRead(VOLTAGE_SENSOR) * (3.3 / 1024.0) * 2.0;
            currentSum += analogRead(CURRENT_SENSOR) * (3.3 / 1024.0) * 50.0;
        }
        float voltage = voltageSum / 5.0;
        float current = currentSum / 5.0;

        voltage_buffer[sample_count] = voltage;
        current_buffer[sample_count] = current;
        power_buffer[sample_count] = voltage * current;

        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(10));
    }
}

void readFlowSensors(void *parameter)
{
    TickType_t lastWakeTime = xTaskGetTickCount();
    for (;;)
    {
        uint32_t count1, count2;

        // Atomically read and reset the flow pulse counters
        portENTER_CRITICAL(&flowMux);
        count1 = flow_frequency_1;
        count2 = flow_frequency_2;
        flow_frequency_1 = 0;
        flow_frequency_2 = 0;
        portEXIT_CRITICAL(&flowMux);

        // Calculate flow in L/min (based on 7.5 pulses per liter)
        int index = sample_count / 100;
        if (index < SAMPLES_FLOW)
        {
            flow_buffer[index][0] = (count1 * 60.0f / PLUSES_PER_LITER);
            flow_buffer[index][1] = (count2 * 60.0f / PLUSES_PER_LITER);
        }

        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(100)); // 10Hz execution
    }
}

void readPressureSensors(void *parameter)
{
    TickType_t lastWakeTime = xTaskGetTickCount();
    for (;;)
    {
        if (xSemaphoreTake(sampleMutex, portMAX_DELAY))
        {
            pressure_buffer[sample_count][0] = bmp1.readPressure() / 100000.0;
            pressure_buffer[sample_count][1] = bmp2.readPressure() / 100000.0;
            pressure_buffer[sample_count][2] = bmp3.readPressure() / 100000.0;
            pressure_buffer[sample_count][3] = bmp4.readPressure() / 100000.0;
            pressure_buffer[sample_count][4] = bmp5.readPressure() / 100000.0;
            pressure_buffer[sample_count][5] = bmp6.readPressure() / 100000.0;
            // in 60 seconds
            //  sample_count -> 0 ---> PS1, PS2, PS3, PS4, PS5, PS6
            //  sample_count -> 1 ---> PS1, PS2, PS3, PS4, PS5, PS6
            //  sample_count -> 2 ---> PS1, PS2, PS3, PS4, PS5, PS6
            //  sample_count -> 3 ---> PS1, PS2, PS3, PS4, PS5, PS6
            //  sample_count -> 4 ---> PS1, PS2, PS3, PS4, PS5, PS6
            //  sample_count -> 5 ---> PS1, PS2, PS3, PS4, PS5, PS6
            //.......
            //  sample_count -> 5999 ---> PS1, PS2, PS3, PS4, PS5, PS6
            xSemaphoreGive(sampleMutex);
        }
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(10));
    }
}

void readTemperatureSensors(void *parameter)
{
    TickType_t lastWakeTime = xTaskGetTickCount();
    for (;;)
    {
        int index = sample_count / 100;
        if (index < SAMPLES_TEMPERATURE)
        {
            float t1 = bmp1.readTemperature();
            float t2 = bmp2.readTemperature();
            float t3 = bmp3.readTemperature();
            float t4 = bmp4.readTemperature();

            temperature_buffer[index][0] = t1;
            temperature_buffer[index][1] = t2;
            temperature_buffer[index][2] = t3;
            temperature_buffer[index][3] = t4;

            // Calculate CE, CP, SE
            float deltaT = ((t1 + t3) / 2.0) - ((t2 + t4) / 2.0);
            float flow = (flow_buffer[index][0] + flow_buffer[index][1]) / 2.0; // avg L/min
            float cp = (deltaT * flow * 4.186) / 60.0;                          // Convert to kW (assuming water)
            float eps = power_buffer[index * 100];
            float ce = (eps > 0) ? (cp / eps) * 100.0 : 0.0;
            float se = ce * 0.9;

            cp_buffer[index] = cp;
            ce_buffer[index] = ce;
            se_buffer[index] = se;
        }
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(1000));
    }
}

void readVibrationSensor(void *parameter)
{
    TickType_t lastWakeTime = xTaskGetTickCount();
    float ax_buffer[VIBRATION_SAMPLE_COUNT];
    float ay_buffer[VIBRATION_SAMPLE_COUNT];
    float az_buffer[VIBRATION_SAMPLE_COUNT];
    int buffer_index = 0;
    int store_index = 0;

    for (;;)
    {
        sensors_event_t event;
        accel.getEvent(&event);
        // Store 100Hz samples
        ax_buffer[buffer_index] = event.acceleration.x;
        ay_buffer[buffer_index] = event.acceleration.y;
        az_buffer[buffer_index] = event.acceleration.z;
        buffer_index++;

        // Once per second (1Hz), calculate RMS and velocity
        if (buffer_index >= VIBRATION_SAMPLE_COUNT)
        {
            float sum = 0;
            for (int i = 0; i < VIBRATION_SAMPLE_COUNT; i++)
            {
                float a_mag = sqrt(
                    ax_buffer[i] * ax_buffer[i] +
                    ay_buffer[i] * ay_buffer[i] +
                    az_buffer[i] * az_buffer[i]);
                sum += a_mag * a_mag;
            }
            float a_rms = sqrt(sum / VIBRATION_SAMPLE_COUNT); // calculating RMS acceleration

            // Convert to velocity using estimated frequency
            float v_rms_mm_per_s = (a_rms / (2.0f * PI * VIBRATION_FREQ_ESTIMATE)) * 1000.0f;

            // Store to vibration_buffer
            if (xSemaphoreTake(sampleMutex, portMAX_DELAY))
            {
                vibration_buffer[store_index] = v_rms_mm_per_s;
                xSemaphoreGive(sampleMutex);
                store_index++;
                if (store_index >= SAMPLES_VIBRATION)
                    store_index = 0;
            }

            buffer_index = 0; // reset for next 100 samples
        }

        // It was neccesary to take readings at 100Hz although in the dataset for vibration sensor it is 1Hz because to convert to mm/s we need to integrate acceleration, which needs more data.
        // Wait 10 ms (100 Hz)
        vTaskDelayUntil(&lastWakeTime, pdMS_TO_TICKS(10));
    }
}

void usb_en()
{
    usb_enable_flag = !usb_enable_flag; // Toggle USB enable state.
}

void loop()
{
    // Required for ESP32 build system, even if unused.
}

void flow1() { flow_frequency_1++; }
void flow2() { flow_frequency_2++; }
