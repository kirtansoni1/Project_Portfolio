#pragma once
/**
  * Helmet_wear_detection_device
  * This Device detects whether a helmet is being worn or not.
  * It connects to a Wi-Fi network to send status updates.
 */

#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_VL6180X.h>
#include <esp_sleep.h>

/* =========================
 *  FEATURE SWITCHES
 * ========================= */
#ifndef ENABLE_NET
#define ENABLE_NET                1    // 0 = no WiFi/HTTP; 1 = enable WiFi + POSTs
#endif

/* =========================
 *  NETWORK / TELEMETRY
 * ========================= */
#ifndef WIFI_SSID
#define WIFI_SSID                 "ANJANSONI_EXT"          // Change this to be the same SSID as your PC
#endif
#ifndef WIFI_PASS
#define WIFI_PASS                 "1234567890"         // Change this to be the same password as your PC
#endif
#ifndef FLASK_URL
#define FLASK_URL                 "http://192.168.0.147:5000/state" // Replace this IP with your Flask server IP, as shown the terminal output
#endif
#ifndef HTTP_TIMEOUT_MS
#define HTTP_TIMEOUT_MS           5000                  
#endif
#ifndef WIFI_CONNECT_BUDGET_MS
#define WIFI_CONNECT_BUDGET_MS    60000                 
#endif
#ifndef REPORT_TRIES
#define REPORT_TRIES              10
#endif
#ifndef REPORT_RETRY_GAP_MS
#define REPORT_RETRY_GAP_MS       100
#endif

// --- Sleep/USB policy ---
#ifndef DEV_NO_SLEEP_BOOT_MS
#define DEV_NO_SLEEP_BOOT_MS   5000   // grace period after boot to keep CPU awake for flashing/monitoring
#endif

#ifndef SLEEP_TIMER_SAFETY_MS
#define SLEEP_TIMER_SAFETY_MS  5      // guardrail to avoid 0ms/us corner cases when sleeping
#endif

/* =========================
 *  MEASUREMENT / POLICY
 * ========================= */
#ifndef THRESH_MM
#define THRESH_MM                 40   // ON if <= THRESH_MM
#endif
#ifndef HYST_MM
#define HYST_MM                   5    // OFF if >= THRESH_MM + HYST_MM
#endif
#ifndef DEBOUNCE_COUNT
#define DEBOUNCE_COUNT            5
#endif
#ifndef SAMPLE_GAP_MS
#define SAMPLE_GAP_MS             200
#endif
#ifndef POLL_PERIOD_SEC
#define POLL_PERIOD_SEC           1.5    // poll cadence
#endif
#ifndef HEARTBEAT_SEC
#define HEARTBEAT_SEC             60   // POST current state at this interval even without transitions
#endif
#ifndef POST_ON_BOOT
#define POST_ON_BOOT              1    // force a POST once on boot
#endif

/* =========================
 *  PINS (XIAO ESP32-C3 defaults)
 * ========================= */
#ifndef I2C_SDA_PIN
#define I2C_SDA_PIN               6
#endif
#ifndef I2C_SCL_PIN
#define I2C_SCL_PIN               7
#endif
#ifndef VL6180X_ADDR
#define VL6180X_ADDR              0x29
#endif

/* =========================
 *  LOGGING
 * ========================= */
#ifndef LOG_ERRORS_ONLY
#define LOG_ERRORS_ONLY           0    // 1 = only errors; 0 = info + errors
#endif

/* =========================
 *  TYPES & GLOBALS
 * ========================= */
enum DeviceState : uint8_t { STATE_OFF = 0, STATE_ON = 1 };
extern Adafruit_VL6180X g_vl6180x;
