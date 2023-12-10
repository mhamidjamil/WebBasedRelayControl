#include "arduino_secrets.h"
#include <ESP8266WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

// Replace with your network credentials
const char *ssid = MY_SSID;
const char *password = MY_PASSWORD;

// Create an instance of the LiquidCrystal_I2C class
LiquidCrystal_I2C lcd(0x27, 16, 2); // 0x27 is the I2C address for the LCD

// Define the pin numbers for the relays
const int relay1Pin = 12; // Change to your desired pin for relay 1
const int relay2Pin = 13; // Change to your desired pin for relay 2

// Variables to store the time to turn on each relay
unsigned long targetTimeRelay1 = 0;
unsigned long targetTimeRelay2 = 0;
// dealing system in minutes instead of milliseconds
// # use getCurrentTimeInMinutes() instead of mills()

String line1, line2;

WiFiServer server(80); // Create a server object

void setup() {
  // Start the serial communication
  Serial.begin(115200);

  // Initialize the LCD
  lcd.begin(16, 2);
  lcd.init();

  // Turn on the backlight
  lcd.backlight();

  // Display connecting to Wi-Fi message
  lcd.clear();
  lcd.setCursor(0, 1);
  lcd.print("Connecting to WiFi...");

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  lcd.clear();
  lcd.setCursor(0, 0);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    lcd.print("Connected");
    Serial.println("Connecting to WiFi...");
  }

  // Display connected to Wi-Fi message
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Connected");
  Serial.println("Connected");

  // Set up the relays as outputs
  pinMode(relay1Pin, OUTPUT);
  pinMode(relay2Pin, OUTPUT);

  // Default: Turn off relays
  digitalWrite(relay1Pin, HIGH);
  digitalWrite(relay2Pin, HIGH);

  server.begin(); // Start the server
}
void loop() {
  // Check if a client has connected
  WiFiClient client = server.available();

  if (client.available()) {

    // Read the first line of the request
    String request = client.readStringUntil('\r');
    Serial.println(request);
    client.flush();

    // Extract hour and minute for relay 1 and relay 2 from the request
    if (request.indexOf("GET /?") != -1) {
      int onHour1 =
          request.substring(request.indexOf("hour1=") + 6, request.indexOf('&'))
              .toInt();
      int onMinute1 =
          request
              .substring(request.indexOf("minute1=") + 8,
                         request.indexOf('&', request.indexOf("minute1=")))
              .toInt();
      int onHour2 =
          request
              .substring(request.indexOf("hour2=") + 6,
                         request.indexOf('&', request.indexOf("hour2=")))
              .toInt();
      int onMinute2 =
          request
              .substring(request.indexOf("minute2=") + 8,
                         request.indexOf('&', request.indexOf("minute2=")))
              .toInt();

      // Check if the extracted values for relay 1 are non-zero
      if (onHour1 > 0 || onMinute1 > 0) {
        targetTimeRelay1 = onHour1 * 60 + onMinute1 + getCurrentTimeInMinutes();
        Serial.print("Relay 1 Time Set: ");
        Serial.print(onHour1);
        Serial.print(" hours ");
        Serial.print(onMinute1);
        Serial.println(" minutes");
      }

      // Check if the extracted values for relay 2 are non-zero
      if (onHour2 > 0 || onMinute2 > 0) {
        targetTimeRelay2 = onHour2 * 60 + onMinute2 + getCurrentTimeInMinutes();
        Serial.print("Relay 2 Time Set: ");
        Serial.print(onHour2);
        Serial.print(" hours ");
        Serial.print(onMinute2);
        Serial.println(" minutes");
      }
    }

    // Send a standard HTTP response header
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();

    // Display the HTML web page
    client.println("<!DOCTYPE HTML>");
    client.println("<html>");
    client.println("<body>");
    client.println("<h1>ESP8266 Web Relay Control</h1>");
    client.println("<form>");
    client.println("Relay 1 Time: Hour: <input type='number' name='hour1' "
                   "min='0' max='99'>");
    client.println(
        "Minute: <input type='number' name='minute1' min='0' max='999'><br>");
    client.println("Relay 2 Time: Hour: <input type='number' name='hour2' "
                   "min='0' max='99'>");
    client.println(
        "Minute: <input type='number' name='minute2' min='0' max='999'><br>");
    client.println("<input type='submit' value='Set Time'>");
    client.println("</form>");
    client.println("</body>");
    client.println("</html>");
  }
  // Turn on relays if the current time matches
  if (getCurrentTimeInMinutes() >=
      targetTimeRelay1) { // means relay should be off
    turnRelay(1, false);
    line1 = "Relay 1 off";
  } else {
    turnRelay(1, true);
    line1 = "Relay 1 On " +
            timeToString(targetTimeRelay1 - getCurrentTimeInMinutes());
  }

  if (getCurrentTimeInMinutes() >=
      targetTimeRelay2) { // means relay should be off
    turnRelay(2, false);
    line2 = "Relay 2 off";
  } else {
    turnRelay(2, true);
    line2 = "Relay 2 On " +
            timeToString(targetTimeRelay2 - getCurrentTimeInMinutes());
  }
  showRelayTiming();
  // Wait a little before responding to the next request
  delay(1000);
  if (millis() < 10000) {
    Serial.print("Time: " + String(getCurrentTimeInMinutes()));
  }
}

void show(int line_number, int column_number, String message) {
  lcd.setCursor(column_number, line_number);
  lcd.print(message);
}

void show(int line_number, String message) {
  lcd.setCursor(0, line_number);
  lcd.print(message);
}

void show(String message) {
  lcd.setCursor(0, 0);
  lcd.print(message);
}

void showRelayTiming() {
  lcd.clear();
  show(0, 0, line1);
  show(1, 0, line2);
  // TODO: Need more optimization
  // if there is not data to show then ask orange-pi pi for time or notification
  // or set display off
}

void turnRelay(int relayNumber, bool state) {
  if (relayNumber == 1) {
    digitalWrite(relay1Pin, !state);
  } else if (relayNumber == 2) {
    digitalWrite(relay2Pin, !state);
  }
}

unsigned int getCurrentTimeInMinutes() { return millis() / 60000; }

String timeToString(unsigned int timeInMinutes) {
  if (timeInMinutes < 60) {
    return String(timeInMinutes) + ":" + String((millis() / 1000) % 60);
  } else if (timeInMinutes > 60 && timeInMinutes < 1440) {
    return (String(timeInMinutes / 60) + ":" + String(timeInMinutes % 60) +
            +":" + String((millis() / 1000) % 60));
  } else {
    return (String(timeInMinutes / 1440) + "," +
            String((timeInMinutes % 1440) / 60) + ":" +
            String((timeInMinutes % 1440) % 60));
  }
}