# Echoes of Space

## Cecilia Hung (2320001)

### Separate Environmental Noise Detection + Interactive Environmental Music Creation Device

Sensing Device: Captures and appropriately filters the ambient noise in the environment, and transmits it to the Display Device. This device can be placed freely in various spaces to trigger the generation of different music by the Display Device.

Display Device: Based on the noise patterns in the environment, generates a short piece of music using a specified algorithm and outputs it.

### Display Device: Illuminator of the Unseen Resonance

- **Primary Design**: Wearable device
- **Iterated Design**: Portable device

### Sensing Device: Whisperer of the Unheard Symphony

- **Function**: Captures and filters ambient noise, transmits to Display Device. Can be placed in various spaces.

### Components

- **Sensing Device**
![Sensing Device](images/sensing.jpg)
  - Microphone: INMP441
  - Bluetooth/Wi-Fi Module: ESP32
  - Battery: 18650 lithium battery LIR18650

- **Display Device**
![Display Device](images/display.jpg)
  - MCU: Arduino Uno or Raspberry Pi 4
  - Bluetooth/Wi-Fi Module: ESP32
  - Battery: 18650 lithium battery LIR18650
  - Speaker: PAM8403
  - LED: Standard 5mm LEDs or 5050 SMD LEDs.

### System Architecture Diagram
![System Architecture](images/diagram.jpg)

