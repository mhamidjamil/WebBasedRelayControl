# ESP8266 Timer Switch

## Overview

This project utilizes the ESP8266 WiFi module to control two relays based on user-defined time settings. It includes a web interface for configuring relay schedules and provides real-time status updates on an LCD screen.

## Features

- Set custom on/off schedules for two relays.
- Web-based configuration interface.
- Real-time status display on a 16x2 LCD.
- WiFi connectivity with automatic network creation if connection fails.

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Setup](#setup)
  - [Hardware Requirements](#hardware-requirements)
  - [Software Requirements](#software-requirements)
  - [Configuration](#configuration)
- [Usage](#usage)
  - [Web Interface](#web-interface)
  - [LCD Display](#lcd-display)
  - [Variable Retrieval](#variable-retrieval)
- [Contributing](#contributing)
- [License](#license)

## Setup

### Hardware Requirements

- ESP8266 board (e.g., NodeMCU)
- 16x2 I2C LCD
- Two-channel relay module
- Resistors, wires, and power supply

### Software Requirements

- [Arduino IDE](https://www.arduino.cc/en/software)
- ESP8266 board support package
- LiquidCrystal_I2C library
- ArduinoJson library

### Configuration

1. Install the required libraries in the Arduino IDE.
2. Open the `uploaded_code.ino` file in the Arduino IDE.
3. Update the `arduino_secrets.h` file with your WiFi credentials.
4. Upload the code to your ESP8266 board.

## Usage

### Web Interface

1. Connect to the WiFi network created by the ESP8266.
2. Open a web browser and navigate to the IP address displayed on the LCD.
3. Set the desired on/off schedules for relay 1 and relay 2.
4. Click "Set Time" to apply the changes.

### LCD Display

The LCD displays real-time information, including charging status and relay schedules.

### Variable Retrieval

To retrieve variable values via a GET request:

- Send a GET request to `http://[ESP_IP_ADDRESS]/get-values`.
- The response will be a JSON object containing:
  - `targetTimeRelay1`: Schedule for relay 1.
  - `targetTimeRelay2`: Schedule for relay 2.
  - `currentTime`: Current time in minutes.

## Contributing

Contributions are welcome! Feel free to open issues and submit pull requests.
