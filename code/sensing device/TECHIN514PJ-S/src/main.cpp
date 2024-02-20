#include <I2S.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"

#define RECORD_TIME   20  // seconds, adjust as needed. Max value is 240
#define WAV_FILE_NAME "arduino_rec" // adjust the file name as needed

#define SAMPLE_RATE   16000U
#define SAMPLE_BITS   16
#define WAV_HEADER_SIZE 44
#define VOLUME_GAIN   2 // adjust the volume gain as needed

void setup() {
  Serial.begin(115200);
  while (!Serial); // Wait for serial port to connect

  // Start I2S with 16 kHz and 16-bits per sample
  I2S.setAllPins(-1, 42, 41, -1, -1);
  if (!I2S.begin(PDM_MONO_MODE, SAMPLE_RATE, SAMPLE_BITS)) {
    Serial.println("Failed to initialize I2S!");
    while (1); // Do nothing if I2S fails to start
  }
  
  // Mount the SD card
  if (!SD.begin(21)) {
    Serial.println("Failed to mount SD Card!");
    while (1); // Do nothing if SD fails to start
  }

  // Record audio to WAV file
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

  // Create and open a WAV file
  File file = SD.open("/" WAV_FILE_NAME ".wav", FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing!");
    return;
  }

  // Write WAV header to the file
  uint8_t wav_header[WAV_HEADER_SIZE];
  generate_wav_header(wav_header, record_size, SAMPLE_RATE);
  file.write(wav_header, WAV_HEADER_SIZE);

  // Start recording and read samples into the buffer
  esp_i2s::i2s_read(esp_i2s::I2S_NUM_0, rec_buffer, record_size, &sample_size, portMAX_DELAY);
  
  // Increase volume and write the buffer to the WAV file
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
  // Code to generate the WAV header based on the size and sample rate
  // Use the code provided in the tutorial to fill this function
  // ...
}

// Optionally add more functions to handle errors or specific tasks
