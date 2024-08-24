# LoRa Sender with Time Synchronization and Sleep Mode

This project implements a LoRa-based sender using the ESP32 microcontroller. The sender is designed to transmit messages via LoRa, synchronize its time with a receiver, and enter deep sleep mode to conserve power. The device wakes up either periodically or due to an external event, sends a message, and goes back to sleep. Additionally, the sender will sleep through the night, waking up at 7 AM to resume operations.

## Features

- **LoRa Communication:** Transmits data packets over LoRa at 915 MHz with ACK-based reliability.
- **Deep Sleep Mode:** Enters deep sleep to save power, waking up either after a set interval or when an external event occurs.
- **Time Synchronization:** Synchronizes time with a receiver to ensure accurate sleep scheduling.
- **Night Mode:** Sleeps from 8 PM to 7 AM, waking up automatically at 7 AM to resume operations.
- **GPIO Wakeup:** Wakes up when a signal is detected on the configured GPIO pin.

## Hardware Requirements

- **ESP32 Development Board**
- **LoRa Transceiver Module** (e.g., SX1276 or similar)
- **External Components:** Button or sensor connected to a GPIO pin for event-based wakeup

## Pin Configuration

- **SS (Slave Select):** GPIO 18
- **RST (Reset):** GPIO 14
- **DIO0 (Interrupt):** GPIO 26
- **WAKE_PIN:** GPIO 2 (for external event wakeup)

## Installation and Setup

1. **Clone the Repository:**
   ```bash
   git clone <repository-url>
   cd LoRa-Sender
