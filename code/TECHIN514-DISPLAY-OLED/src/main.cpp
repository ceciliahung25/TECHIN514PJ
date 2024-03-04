#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <SwitecX25.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1    // Define OLED reset pin (may not be needed for your display)
#define STEPS 945        // Define the steps for the motor

// Motor pin definitions
const int motorPin1 = 1;
const int motorPin2 = 2;
const int motorPin3 = 3;
const int motorPin4 = 4;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
SwitecX25 motor(STEPS, motorPin1, motorPin2, motorPin3, motorPin4);

const char* ssid = "ESP32_Sensing_Device";
const char* password = "yourPassword";
const char* serverIP = "192.168.4.1";
const uint16_t serverPort = 1234;

WiFiClient client;

const int maxDataPoints = 20;
float rmsData[maxDataPoints];
int dataIndex = 0;
float currentRMS = 0.0;

void drawHistogram();
void updateMotorPosition(float rmsValue);

void setup(void) {
    Serial.begin(9600);
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
        for (;;);
    }
    display.display();
    delay(2000);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    // Initialize the rmsData array
    for (int i = 0; i < maxDataPoints; i++) {
        rmsData[i] = 0.0;
    }

    // Initialize the motor
    motor.zero();
}

void loop(void) {
    static float lastRMS = -1.0;

    if (!client.connected()) {
        if (!client.connect(serverIP, serverPort)) {
            delay(1000);
            return;
        }
        client.println("Request data");
    }

    if (client.available()) {
        String line = client.readStringUntil('\n');
        if (line.length() > 0 && line != "\r") {
            int commaIndex = line.indexOf(',');
            currentRMS = line.substring(0, commaIndex).toFloat();

            if (currentRMS != lastRMS) {
                lastRMS = currentRMS;

                // Store RMS data and update the index
                rmsData[dataIndex] = currentRMS;
                dataIndex = (dataIndex + 1) % maxDataPoints;

                // Update OLED display
                display.clearDisplay();
                display.setTextSize(1);
                display.setTextColor(WHITE);
                display.setCursor(0, 0);
                display.print("RMS Volume: ");
                display.println(currentRMS);

                // Draw histogram
                drawHistogram();

                // Update motor position
                updateMotorPosition(currentRMS);

                display.display();
            }
        }
    }

    // Update the motor position
    motor.update();
}

void drawHistogram() {
    int barWidth = SCREEN_WIDTH / maxDataPoints;
    for (int i = 0; i < maxDataPoints; i++) {
        int barHeight = map(rmsData[(dataIndex + i) % maxDataPoints], 0, 25, 0, SCREEN_HEIGHT / 2);
        display.drawRect(SCREEN_WIDTH - (i + 1) * barWidth, SCREEN_HEIGHT - barHeight, barWidth - 1, barHeight, WHITE);
    }
}

void updateMotorPosition(float rmsValue) {
    int targetPosition;
    if (rmsValue >= 0 && rmsValue <= 15) {
        targetPosition = STEPS / 3; // 1/3 full rotation
    } else if (rmsValue > 15 && rmsValue <= 20) {
        targetPosition = (2 * STEPS) / 3; // 2/3 full rotation
    } else if (rmsValue > 20) {
        targetPosition = STEPS - 1; // Full rotation
    }
    motor.setPosition(targetPosition);
}
