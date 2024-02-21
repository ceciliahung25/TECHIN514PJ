#include <I2S.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"

// Define constants
#define RECORD_TIME   20  // Recording time in seconds
#define WAV_FILE_NAME "arduino_rec.wav" // Name of the WAV file
#define SAMPLE_RATE   16000U // Sample rate in Hz
#define SAMPLE_BITS   16 // Bit depth
#define WAV_HEADER_SIZE 44
#define VOLUME_GAIN   2

// Function prototypes
void record_wav();
void generate_wav_header(uint8_t *header, uint32_t size, uint32_t sampleRate);

void setup() {
  Serial.begin(115200);
  while (!Serial); // Wait for serial port to connect

  // Attempt a generic initialization of the I2S interface
  if (!I2S.begin(SAMPLE_RATE, SAMPLE_BITS)) {
    Serial.println("Failed to initialize I2S!");
    while (1); // Stop if failed
  }

  // Initialize SD card
  if (!SD.begin(21)) { // Adjust the pin number for your SD card interface
    Serial.println("Failed to mount SD card!");
    while (1); // Stop if failed
  }

  // Start recording
  record_wav();
}


void loop() {
  delay(1000);
  Serial.print(".");
}

void record_wav() {
  uint32_t sample_size = 0;
  uint32_t record_size = SAMPLE_RATE * SAMPLE_BITS / 8 * RECORD_TIME;
  uint8_t *rec_buffer = (uint8_t *)ps_malloc(record_size);

  if (rec_buffer == NULL) {
    Serial.println("Memory allocation failed!");
    while (1);
  }

  File file = SD.open(WAV_FILE_NAME, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing!");
    return;
  }

  uint8_t wav_header[WAV_HEADER_SIZE];
  generate_wav_header(wav_header, record_size, SAMPLE_RATE);
  file.write(wav_header, WAV_HEADER_SIZE);

  esp_i2s::i2s_read(esp_i2s::I2S_NUM_0, rec_buffer, record_size, &sample_size, portMAX_DELAY);

  for (uint32_t i = 0; i < sample_size; i += SAMPLE_BITS / 8) {
    int16_t *sample = (int16_t *)(rec_buffer + i);
    *sample = *sample << VOLUME_GAIN;
  }

  file.write(rec_buffer, sample_size);
  file.close();
  free(rec_buffer);

  Serial.println("Recording finished.");
}

void generate_wav_header(uint8_t *header, uint32_t size, uint32_t sampleRate) {
  // Add your code here to generate the WAV header
  // ...
}

// Additional functions can be added here
