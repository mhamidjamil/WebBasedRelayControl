//$ last work 10/December/23 [12:27 AM]
// # version 0.5
// # Release Note : Permanently on/off switch
#include "arduino_secrets.h"
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>

// Replace with your network credentials
const char *ssid = MY_SSID;
const char *password = MY_PASSWORD;

// Create an instance of the LiquidCrystal_I2C class
LiquidCrystal_I2C lcd(0x27, 16, 2); // 0x27 is the I2C address for the LCD

#define WIFI_CONNECTION_TIMEOUT 15 // 15 seconds

// Define the pin numbers for the relays
const int relay1Pin = 12;   // Change to your desired pin for relay 1
const int relay2Pin = 13;   // Change to your desired pin for relay 2
const int chargingPin = 16; // D0 pin on NodeMCU
bool chargingState = false;

// Variables to store the time to turn on each relay
unsigned long targetTimeRelay1 = 0;
unsigned long targetTimeRelay2 = 0;
// dealing system in minutes instead of milliseconds
// # use getCurrentTimeInMinutes() instead of mills()
// FIXME: Dealing in minutes cost:
// if you want to off relay after 1 minute and the current system time is:
// in seconds => 23 seconds then you relay will be off in 37 seconds instead of
// 60 seconds

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
  while (WiFi.status() != WL_CONNECTED &&
         (millis() / 1000) < WIFI_CONNECTION_TIMEOUT) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
    lcd.setCursor(0, 0);
    lcd.print("Connecting...");
  }

  // Check if connected
  if (WiFi.status() == WL_CONNECTED) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Connected");
    Serial.println("Connected");
  } else {
    // If not connected within the timeout, create its own network
    createOwnNetwork();
  }

  // Set up the relays as outputs
  pinMode(relay1Pin, OUTPUT);
  pinMode(relay2Pin, OUTPUT);

  // Default: Turn off relays
  digitalWrite(relay1Pin, HIGH);
  digitalWrite(relay2Pin, HIGH);

  server.begin(); // Start the server
  pinMode(chargingPin, INPUT);
}

void loop() {
  // Check if a client has connected
  WiFiClient client = server.available();

  if (client.available()) {

    // Read the first line of the request
    String request = client.readStringUntil('\r');
    Serial.println(request);
    if (request.indexOf("GET /get-values") != -1) {
      handleGetValues(client);
    } else {
      client.flush();
    }
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
      if (onHour1 > -2 || onMinute1 > -2) {
        if (onHour1 == -1 || onMinute1 == -1)
          targetTimeRelay1 = 0;
        else if (onHour1 == 998 || onMinute1 == 998)
          targetTimeRelay1 = 998;
        else if (onHour1 > 0 || onMinute1 > 0) {
          targetTimeRelay1 =
              onHour1 * 60 + onMinute1 + getCurrentTimeInMinutes();
        }
        Serial.print("Relay 1 Time Set: ");
        Serial.print(onHour1);
        Serial.print(" hours ");
        Serial.print(onMinute1);
        Serial.println(" minutes");
      }

      // Check if the extracted values for relay 2 are non-zero
      if (onHour2 > -2 || onMinute2 > -2) {
        if (onHour2 == -1 || onMinute2 == -1)
          targetTimeRelay2 = 0;
        else if (onHour2 == 998 || onMinute2 == 998)
          targetTimeRelay2 = 998;
        else if (onHour2 > 0 || onMinute2 > 0) {
          targetTimeRelay2 =
              onHour2 * 60 + onMinute2 + getCurrentTimeInMinutes();
        }
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
    client.println("<head>");
    client.println("<script>");
    client.println("function forceOn(relayNumber) {");
    client.println(
        "  document.getElementById('minute' + relayNumber).value = 998;");
    client.println("  return false;"); // Prevent form submission
    client.println("}");
    client.println("function forceOff(relayNumber) {");
    client.println(
        "  document.getElementById('minute' + relayNumber).value = -1;");
    client.println("  return false;"); // Prevent form submission
    client.println("}");
    client.println("</script>");
    client.println("</head>");
    client.println("<body>");
    client.println("<h1>ESP8266 Web Relay Control</h1>");
    client.println("<form>");

    // Relay 1
    client.println("Relay 1 Time: Hour: <input type='number' name='hour1' "
                   "min='-1' max='999'>");
    client.println("Minute: <input type='number' id='minute1' name='minute1' "
                   "min='-1' max='999'>");
    client.println(
        "<button type='submit' onclick='forceOn(1)'>Force On</button>");
    client.println(
        "<button type='submit' onclick='forceOff(1)'>Force Off</button><br>");

    // Relay 2
    client.println("Relay 2 Time: Hour: <input type='number' name='hour2' "
                   "min='-1' max='999'>");
    client.println("Minute: <input type='number' id='minute2' name='minute2' "
                   "min='-1' max='999'>");
    client.println(
        "<button type='submit' onclick='forceOn(2)'>Force On</button>");
    client.println(
        "<button type='submit' onclick='forceOff(2)'>Force Off</button><br>");

    client.println("<input type='submit' value='Set Time'>");
    client.println("</form>");
    client.println("</body>");
    client.println("</html>");
  }

  // Turn on relays if the current time matches
  if (getCurrentTimeInMinutes() >= targetTimeRelay1 ||
      targetTimeRelay1 != 998) { // means relay should be off
    turnRelay(1, false);
    line1 = "Relay 1 off";
  } else {
    turnRelay(1, true);
    targetTimeRelay1 == 998
        ? line1 = "Relay 1 ON."
        : line1 = "Relay 1 on " +
                  timeToString(targetTimeRelay1 - getCurrentTimeInMinutes());
  }

  if (getCurrentTimeInMinutes() >= targetTimeRelay2 ||
      targetTimeRelay2 != 998) { // means relay should be off
    turnRelay(2, false);
    line2 = "Relay 2 off";
  } else {
    turnRelay(2, true);
    targetTimeRelay2 == 998
        ? line2 = "Relay 2 ON."
        : line2 = "Relay 2 on " +
                  timeToString(targetTimeRelay2 - getCurrentTimeInMinutes());
  }
  showRelayTiming();
  // Wait a little before responding to the next request
  int chargingStatus = digitalRead(chargingPin);

  if (chargingStatus == HIGH && !chargingState) {
    Serial.println("D0 pin is HIGH");
    chargingState = !chargingState;
  } else if (chargingStatus == LOW && chargingState) {
    Serial.println("D0 pin is LOW");
    chargingState = !chargingState;
  }

  delay(1000);
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
  int chargingStatus = digitalRead(chargingPin);

  if (chargingStatus == HIGH) {
    // if charging and at least one or no relay is scheduled
    if (line1.indexOf("off") != -1 &&
        line2.indexOf("off") != -1) { // in case both relays are off
      line2 = "Relays are off";
    } else if (line1.indexOf("on") != -1 && line2.indexOf("on") != -1) {
      // if charging and both relays are scheduled
      line2 = (targetTimeRelay1 == 998
                   ? "R1:ON"
                   : "R1: " + timeToString(targetTimeRelay1 -
                                           getCurrentTimeInMinutes())) +
              "," +
              (targetTimeRelay2 == 998
                   ? "R2:ON"
                   : "R2: " + timeToString(targetTimeRelay2 -
                                           getCurrentTimeInMinutes()));
    } else if (line1.indexOf("on") != -1 || line2.indexOf("on") != -1) {
      // in case if only one relay in scheduled
      line1.indexOf("on") != -1
          ? (targetTimeRelay1 == 998 ? line2 = "R1:ON" : line2 = line1)
      : targetTimeRelay2 == 998 ? line2 = "R2:ON"
                                : line2 = line2;
    }
    line1 = "Charging...";
  } else if (line1.indexOf("on") != -1 && line2.indexOf("on")) {
    // default case, both relays are scheduled and not charging
    //` no need to be change
  } else if (line1.indexOf("off") != -1 && line2.indexOf("off") != -1) {
    // if both relays are not scheduled and not charging
    // #will work in future
  }
  show(0, 0, line1);
  show(1, 0, line2);
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

void handleGetValues(WiFiClient client) {
  // Create a JSON document to store values
  DynamicJsonDocument doc(1024);

  // Add the variables you want to retrieve
  doc["targetTimeRelay1"] = targetTimeRelay1;
  doc["targetTimeRelay2"] = targetTimeRelay2;
  doc["currentTime"] = getCurrentTimeInMinutes();

  // Serialize the JSON document
  String response;
  serializeJson(doc, response);

  // Send the response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Connection: close");
  client.println();
  client.println(response);
}

void createOwnNetwork() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Creating Network");

  // Set up your own network
  // Configure the SoftAP (Access Point)
  const char *ap_ssid = "TimerSwitch";
  const char *ap_password = "Password@!";
  WiFi.softAP(ap_ssid, ap_password);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("IP: ");
  lcd.setCursor(3, 1);
  lcd.print(WiFi.softAPIP());
  Serial.println(WiFi.softAPIP());
  delay(2000);
  Serial.println("Own network created");
}
