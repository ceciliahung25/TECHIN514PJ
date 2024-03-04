#include <Arduino.h>
#include <WiFi.h>
#include <I2S.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"

#define BUTTON_PIN D7
#define RECORD_TIME 5
#define WAV_FILE_NAME "/arduino_rec.wav"
#define SAMPLE_RATE 16000U
#define SAMPLE_BITS 16
#define WAV_HEADER_SIZE 44
#define ALPHA 0.1 // Low-pass filter constant (0 < ALPHA < 1)

const char* ssid = "ESP32_Sensing_Device";
const char* password = "yourPassword";
WiFiServer server(1234);

float rmsValue = 0.0;
float zcrValue = 0.0;

float calculateRMS(const uint8_t* buffer, size_t bufferSize);
float calculateZCR(const uint8_t* buffer, size_t bufferSize);
void generate_wav_header(uint8_t *header, uint32_t size, uint32_t rate);
void record_wav();
float lowPassFilter(float currentValue, float newValue);

void setup() {
    Serial.begin(115200);
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    // Initialize I2S
    I2S.setAllPins(-1, 42, 41, -1, -1);
    if (!I2S.begin(PDM_MONO_MODE, SAMPLE_RATE, SAMPLE_BITS)) {
        Serial.println("Failed to initialize I2S!");
        while (1);
    }

    // Initialize SD card
    if (!SD.begin(21)) {
        Serial.println("Failed to mount SD card!");
        while (1);
    }

    // Set up Wi-Fi network
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);
    Serial.println("Wi-Fi Access Point started");
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());

    // Start the server
    server.begin();
    Serial.println("Server started");
}

void loop() {
    if (digitalRead(BUTTON_PIN) == LOW) {
        delay(50);
        if (digitalRead(BUTTON_PIN) == LOW) {
            Serial.println("Button pressed, starting recording...");
            record_wav();
        }
    }

    // Check for Wi-Fi clients
    WiFiClient client = server.available();
    if (!client) {
        return;
    }

    Serial.println("Client connected");
    while (client.connected()) {
        if (client.available()) {
            String request = client.readStringUntil('\n');
            if (request.indexOf("Request data") >= 0) {
                String dataToSend = String(rmsValue) + "," + String(zcrValue);
                client.println(dataToSend);
                Serial.println("Data sent over Wi-Fi: " + dataToSend);
            }
            client.stop();
            Serial.println("Client disconnected");
        }
    }
}

void record_wav() {
    Serial.println("Starting recording...");
    uint32_t record_size = (SAMPLE_RATE * SAMPLE_BITS / 8) * RECORD_TIME;
    uint8_t *rec_buffer = (uint8_t *)ps_malloc(record_size);
    if (rec_buffer == NULL) {
        Serial.println("Memory allocation failed!");
        return;
    }

    // Reset filter values before new recording
    rmsValue = 0.0;
    zcrValue = 0.0;

    File file = SD.open(WAV_FILE_NAME, FILE_WRITE);
    if (!file) {
        Serial.println("Failed to open file for writing.");
        free(rec_buffer);
        return;
    }

    uint8_t wav_header[WAV_HEADER_SIZE];
    generate_wav_header(wav_header, record_size, SAMPLE_RATE);
    file.write(wav_header, WAV_HEADER_SIZE);

    size_t bytes_read = I2S.readBytes(rec_buffer, record_size);
    Serial.print("Bytes read: ");
    Serial.println(bytes_read);

    if (bytes_read > 0) {
        float newRMS = calculateRMS(rec_buffer, bytes_read);
        float newZCR = calculateZCR(rec_buffer, bytes_read);

        rmsValue = lowPassFilter(rmsValue, newRMS);
        zcrValue = lowPassFilter(zcrValue, newZCR);

        Serial.print("RMS Volume: ");
        Serial.println(rmsValue);
        Serial.print("Zero Crossing Rate (Frequency): ");
        Serial.println(zcrValue);
    } else {
        Serial.println("No data recorded.");
    }

    file.write(rec_buffer, bytes_read);
    file.close();
    free(rec_buffer);
}

float calculateRMS(const uint8_t* buffer, size_t bufferSize) {
    long sum = 0;
    for (size_t i = 0; i < bufferSize; i += 2) {
        int16_t sample = (int16_t)(buffer[i] | (buffer[i + 1] << 8));
        sum += sample * sample;
    }
    return sqrt(sum / (bufferSize / 2));
}

float calculateZCR(const uint8_t* buffer, size_t bufferSize) {
    int zeroCrossings = 0;
    for (size_t i = 2; i < bufferSize; i += 2) {
        int16_t sample1 = (int16_t)(buffer[i - 2] | (buffer[i - 1] << 8));
        int16_t sample2 = (int16_t)(buffer[i] | (buffer[i + 1] << 8));
        if ((sample1 > 0 && sample2 < 0) || (sample1 < 0 && sample2 > 0)) {
            zeroCrossings++;
        }
    }
    return (float)zeroCrossings * SAMPLE_RATE / bufferSize;
}

void generate_wav_header(uint8_t *header, uint32_t size, uint32_t rate) {
    // [Your existing generate_wav_header function]
}

float lowPassFilter(float currentValue, float newValue) {
    return ALPHA * newValue + (1 - ALPHA) * currentValue;
}
