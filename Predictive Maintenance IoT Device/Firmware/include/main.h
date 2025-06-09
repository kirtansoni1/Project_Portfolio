#ifndef MAIN_H
#define MAIN_H

#include "defines.h"
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>
#include <Adafruit_ADXL345_U.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#ifdef __cplusplus
extern "C"
{
#endif

    // ----------------------------------
    // Global Sensor Objects
    // ----------------------------------

    /**
     * @brief Array of BMP280 pressure sensors (SPI mode).
     */
    extern Adafruit_BMP280 bmp280_sensors[6];

    /**
     * @brief ADXL345 accelerometer object (I2C).
     */
    extern Adafruit_ADXL345_Unified accel;

    // ----------------------------------
    // Flow Sensor Interrupt Counters
    // ----------------------------------

    /**
     * @brief Counts flow pulses from sensor 1 (used in ISR).
     */
    extern volatile uint32_t flow_frequency_1;

    /**
     * @brief Counts flow pulses from sensor 2 (used in ISR).
     */
    extern volatile uint32_t flow_frequency_2;

    // ----------------------------------
    // Sensor Data Buffers
    // ----------------------------------

    extern float pressure_buffer[SAMPLES_PRESSURE][NUM_OF_PRESSURE_SENSORS];
    extern float voltage_buffer[SAMPLES_VOLTAGE];
    extern float current_buffer[SAMPLES_CURRENT];
    extern float flow_buffer[SAMPLES_FLOW][NUM_OF_FLOW_SENSORS];
    extern float temperature_buffer[SAMPLES_TEMPERATURE][NUM_OF_TEMP_SENSORS];
    extern float vibration_buffer[SAMPLES_VIBRATION];

    // ----------------------------------
    // Global Sample Index and Mutex
    // ----------------------------------

    /**
     * @brief Sample index counter (0–SAMPLES_PRESSURE), incremented every 10ms.
     */
    extern int sample_count;

    /**
     * @brief Mutex to synchronize access to sample buffers.
     */
    extern SemaphoreHandle_t sampleMutex;

    // ----------------------------------
    // Function Declarations
    // ----------------------------------

    /**
     * @brief Initializes all connected sensors.
     */
    void initializeSensors();

    /**
     * @brief Task: Reads 6 BMP280 pressure sensors at 100 Hz.
     */
    void readPressureSensors(void *parameter);

    /**
     * @brief Task: Reads voltage and current sensors at 100 Hz and averages 5 samples.
     */
    void readVoltageCurrentSensors(void *parameter);

    /**
     * @brief Task: Reads flow rate from pulse counters at 10 Hz and resets pulse counters.
     */
    void readFlowSensors(void *parameter);

    /**
     * @brief Task: Reads temperature from BMP280 sensors at 1 Hz.
     */
    void readTemperatureSensors(void *parameter);

    /**
     * @brief Task: Collects acceleration data from ADXL345 at 100 Hz,
     * calculates RMS vibration velocity (mm/s), and stores it at 1 Hz.
     */
    void readVibrationSensor(void *parameter);

    /**
     * @brief Calculates and transmits all buffered sensor data over Serial when buffer is full.
     */
    void transmitSensorData();

    /**
     * @brief Task: Increments sample counter at 100 Hz. Resets and triggers data transmission at max sample count.
     */
    void incrementSampleCount(void *parameter);

    /**
     * @brief Interrupt handler for flow sensor 1 pulse input.
     */
    void flow1();

    /**
     * @brief Interrupt handler for flow sensor 2 pulse input.
     */
    void flow2();

    /**
     * @brief Interrupt handler for usb en button.
     */
    void usb_en();

#ifdef __cplusplus
}
#endif

#endif // MAIN_H
