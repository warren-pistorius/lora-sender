# ESP32 LoRa Communication with OTA Update and Deep Sleep

This project demonstrates the use of an ESP32 microcontroller to communicate via LoRa, perform OTA updates, and utilize deep sleep for power saving. The device can wake up periodically, send messages, receive acknowledgments, and go back to sleep. Additionally, it can be put into OTA mode for firmware updates via a button press.

## Features

- **LoRa Communication**: Sends and receives messages using the LoRa protocol.
- **OTA Updates**: Allows Over-The-Air updates to the ESP32 firmware by pressing a button.
- **Deep Sleep Mode**: Utilizes deep sleep to save power between operations.
- **Time Synchronization**: Periodically synchronizes the internal clock to minimize drift.
- **Wakeup Sources**: Can wake up on button press, external pin change, or timer.

## Hardware Setup

- **ESP32**: The main microcontroller used in the project.
- **LoRa Module**: Communicates using LoRa (Long Range) radio technology.
- **Button (GPIO 15)**: Used to enter OTA mode.
- **LED (GPIO 25)**: Indicates the device status (e.g., OTA mode).
- **External Wakeup Pin (GPIO 2)**: Another source for waking up the device.

## Pin Configuration

- **LoRa Pins**:
  - `ss`: GPIO 18
  - `rst`: GPIO 14
  - `dio0`: GPIO 26

- **Button Pin**: GPIO 15 (configured as `buttonPin`)

- **LED Pin**: GPIO 25 (configured as `ledPin`)

- **External Wakeup Pin**: GPIO 2 (configured as `wakePin`)

## Functionality Overview

### 1. LoRa Communication

The ESP32 initializes the LoRa module and communicates with another LoRa device. It sends messages and waits for an acknowledgment (ACK) within a specified timeout period. If the ACK is not received, it retries up to a maximum number of times.

### 2. OTA Mode

If the ESP32 is woken up by a button press, it enters OTA mode. In this mode, the device connects to a predefined Wi-Fi network, allowing firmware updates via the Arduino IDE.

### 3. Deep Sleep

The ESP32 enters deep sleep after completing its tasks to save power. It can be woken up by:

- A button press (`buttonPin`).
- A signal on the external wakeup pin (`wakePin`).
- A timer interrupt (e.g., hourly wakeup for regular check-ins).

### 4. Time Synchronization

To prevent time drift, the device synchronizes its internal clock with an external source once a day. This is done by sending a time request to a receiver and updating the ESP32's clock.

## How to Use

1. **Clone the Repository**: Clone this project to your local machine.
2. **Setup Your Hardware**: Connect the ESP32, LoRa module, and other components as described above.
3. **Upload the Code**: Use the Arduino IDE to upload the code to your ESP32.
4. **Trigger OTA Mode**: Press the button connected to `buttonPin` to enter OTA mode and update the firmware.
5. **Monitor Serial Output**: Use a serial monitor to view log messages and ensure the device is functioning as expected.

## Configuration

You can modify the following constants in the code to suit your requirements:

- **WiFi Credentials**: Update the `WiFi.begin("SSID", "PASSWORD");` line in the `startOTAMode()` function with your network credentials.
- **Sync Interval**: Adjust the `DAILY_SYNC_INTERVAL` to change how often the time is synchronized.
- **Message Retry Count**: Modify `maxRetries` to change the number of times the device retries sending a message.
- **Acknowledgment Timeout**: Adjust `ackTimeout` to change how long the device waits for an ACK.

## Dependencies

- **ArduinoOTA**: For OTA updates.
- **LoRa**: For LoRa communication.
- **esp_sleep**: For deep sleep functionality.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Thanks to the open-source community for the Arduino libraries used in this project.