// This file is only for testing purpose, to test the data stream copy this code to the main.cpp and comment out this file.

#include "main.h"

bool usb_enable_flag = true;

void usb_en();

void setup()
{
    Serial.begin(BAUD_RATE);
    pinMode(USB_EN_BUTTN_PIN, INPUT);
    pinMode(USB_EN_LED_PIN, OUTPUT);
    attachInterrupt(digitalPinToInterrupt(USB_EN_BUTTN_PIN), usb_en, FALLING);
    delay(2000); // Allow time for serial monitor to open
}

void loop()
{
    if (usb_enable_flag)
    {
        digitalWrite(USB_EN_LED_PIN, HIGH);
        Serial.println("START");

        // Simulate and send 100Hz pressure and power (EPS1) data (6000 samples)
        for (int i = 0; i < 6000; i++)
        {
            float t = i / 100.0;
            float voltage = 12.0 + sin(t);
            float current = 0.5 + cos(t);
            float power = voltage * current;

            Serial.printf("PS1:%.2f|PS2:%.2f|PS3:%.2f|PS4:%.2f|PS5:%.2f|PS6:%.2f|EPS1:%.2f\n",
                          1.0 + sin(t), 1.1 + cos(t), 1.2 + sin(t + 1), 1.3 + cos(t + 1),
                          1.4 + sin(t + 2), 1.5 + cos(t + 2),
                          power);
        }

        // Simulate and send 10Hz flow data (600 samples)
        for (int i = 0; i < 600; i++)
        {
            float f = i / 10.0;
            Serial.printf("FS1:%.2f|FS2:%.2f\n", 1.5 + sin(f), 2.5 + cos(f));
        }

        // Simulate and send 1Hz temperature, vibration, and virtual metrics (60 samples)
        for (int i = 0; i < 60; i++)
        {
            float k = i * 1.0;

            // Temperature
            Serial.printf("TS1:%.2f|TS2:%.2f|TS3:%.2f|TS4:%.2f\n",
                          25.0 + sin(k), 25.5 + cos(k),
                          26.0 + sin(k + 1), 26.5 + cos(k + 1));

            // Vibration
            Serial.printf("VS1:%.2f\n", 1.0 + 0.1 * sin(k));

            // Virtual Metrics: CP (kW), CE (%), SE (%)
            float CP = 5.0 + 0.5 * sin(k);        // Example: Cooling power in kW
            float CE = 85.0 + 5.0 * cos(k);       // Example: Cooling efficiency in %
            float SE = 90.0 + 4.0 * sin(k + 0.5); // Example: Efficiency factor in %

            Serial.printf("CP:%.2f|CE:%.2f|SE:%.2f\n", CP, CE, SE);
        }

        Serial.println("END");

        delay(1000); // Wait 1 second before sending next batch
    }
    else
    {
        Serial.println("USB OFF!");
        digitalWrite(USB_EN_LED_PIN, LOW);
        delay(1000);
    }
}

void usb_en()
{
    usb_enable_flag = !usb_enable_flag; // Toggle USB enable state.
}