# ESP8266 Web Relay Control

## Overview

This project enables you to control relays connected to an ESP8266 module through a web interface. You can set specific times for the relays to turn on and off.

## Hardware Requirements

- ESP8266 module
- 2-channel relay module
- Liquid Crystal Display (LCD) with I2C interface
- Resistors, wires, etc.

## Software Requirements

- Arduino IDE
- ESP8266 board support package
- Libraries: LiquidCrystal_I2C

## Getting Started

1. Clone or download this repository.

2. Open the project in the Arduino IDE.

3. Install the required libraries through the Arduino Library Manager.

4. Update the `arduino_secrets.h` file with your WiFi credentials.

5. Upload the code to your ESP8266 module.

6. Open the Serial Monitor to view debug information.

7. Connect to the WiFi network created by your ESP8266.

8. Access the web interface through your browser to set relay timings.

## Usage

- Visit the ESP8266's IP address in your browser to set relay timings.

- Relay 1: [IP_ADDRESS]/?hour1=[HOUR]&minute1=[MINUTE]

- Relay 2: [IP_ADDRESS]/?hour2=[HOUR]&minute2=[MINUTE]

## API (Task 3)

To programmatically set relay timings, send a POST request to the following endpoint:

- Endpoint: [IP_ADDRESS]/set-timings

- Payload: JSON format
  ```json
  {
    "relay1": {
      "hour": 12,
      "minute": 30
    },
    "relay2": {
      "hour": 15,
      "minute": 45
    }
  }
  ```
