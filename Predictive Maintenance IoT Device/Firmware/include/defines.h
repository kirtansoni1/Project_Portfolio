#ifndef DEFINES_H
#define DEFINES_H

// Serial Communication
#define BAUD_RATE 250000

// Sensor Pins
#define FLOW_SENSOR_1 4
#define FLOW_SENSOR_2 5
#define VOLTAGE_SENSOR 6
#define CURRENT_SENSOR 7

// SPI Pins
#define MOSI_PIN 11
#define MISO_PIN 13
#define SCK_PIN 12

// I2C Pins
#define I2C_SDA 9
#define I2C_SCL 10

// Chip Select Pins for BMP280 Sensors
const int BMP_CS_PINS[6] = {1, 2, 41, 40, 39, 38};

// USB EN pins
#define USB_EN_BUTTN_PIN 3
#define USB_EN_LED_PIN 15

// Sampling Rates and Buffers
#define SAMPLES_PRESSURE 6000  // 100Hz for 60 seconds
#define SAMPLES_VOLTAGE 6000   // 100Hz for 60 seconds
#define SAMPLES_CURRENT 6000   // 100Hz for 60 seconds
#define SAMPLES_POWER 6000     // 100Hz for 60 seconds
#define SAMPLES_FLOW 600       // 10Hz for 60 seconds
#define SAMPLES_TEMPERATURE 60 // 1Hz for 60 seconds
#define SAMPLES_VIBRATION 60   // 1Hz for 60 seconds
#define SAMPLES_CE 60          // 1Hz for 60 seconds
#define SAMPLES_SE 60          // 1Hz for 60 seconds
#define SAMPLES_CP 60          // 1Hz for 60 seconds

// Number of sensors
#define NUM_OF_PRESSURE_SENSORS 6
#define NUM_OF_TEMP_SENSORS 4
#define NUM_OF_FLOW_SENSORS 2

// Vibration sensor variables
#define VIBRATION_SAMPLING_HZ 100
#define VIBRATION_FREQ_ESTIMATE 50.0f // Hz
#define VIBRATION_SAMPLE_COUNT (100)  // 1 second at 100 Hz

// Flow sensor variables
#define PLUSES_PER_LITER 7.5f // Pulses the flow sensor outputs when one liter is passed through it
#endif
