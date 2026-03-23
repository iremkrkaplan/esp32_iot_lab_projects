// Enhanced ESP32 Web Server Example
// - Automatic Wi-Fi reconnection
// - Clearer serial messages
// - Simple web control for two LEDs
// - LED1 -> GPIO 4
// - LED2 -> GPIO 18

#include <WiFi.h>

// Your fixed network credentials
const char* ssid = "Erdinc's S21 FE";
const char* password = "123456789";

// Create a web server on port 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Variables to store the current LED states
String led1State = "off";
String led2State = "off";

// Assign output variables to GPIO pins
const int led1 = 4;    // LED1 -> GPIO 4
const int led2 = 18;   // LED2 -> GPIO 18

// Timing variables for client timeout
unsigned long currentTime = 0;
unsigned long previousTime = 0;
const long timeoutTime = 2000;  // 2 seconds

void connectToWiFi() {
  Serial.print("Connecting to Wi-Fi: ");
  Serial.println(ssid);

  WiFi.disconnect(true);
  delay(500);
  WiFi.begin(ssid, password);

  int attempts = 0;
  const int maxAttempts = 20;   // 20 x 500 ms = 10 seconds

  while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Wi-Fi connected successfully.");
    Serial.print("ESP32 IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("Wi-Fi connection failed.");
    Serial.println("Please check hotspot status, signal strength, or credentials.");
  }
}

void sendWebPage(WiFiClient& client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println("Connection: close");
  client.println();

  client.println("<!DOCTYPE html><html>");
  client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
  client.println("<link rel=\"icon\" href=\"data:,\">");

  // CSS style
  client.println("<style>");
  client.println("html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center; }");
  client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
  client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer; }");
  client.println(".button2 { background-color: #555555; }");
  client.println("</style></head>");

  // Page content
  client.println("<body><h1>ESP32 Web Server</h1>");

  // LED1
  client.println("<p>LED 1 (GPIO 4) - State: " + led1State + "</p>");
  if (led1State == "off") {
    client.println("<p><a href=\"/led1/on\"><button class=\"button\">ON</button></a></p>");
  } else {
    client.println("<p><a href=\"/led1/off\"><button class=\"button button2\">OFF</button></a></p>");
  }

  // LED2
  client.println("<p>LED 2 (GPIO 18) - State: " + led2State + "</p>");
  if (led2State == "off") {
    client.println("<p><a href=\"/led2/on\"><button class=\"button\">ON</button></a></p>");
  } else {
    client.println("<p><a href=\"/led2/off\"><button class=\"button button2\">OFF</button></a></p>");
  }

  client.println("</body></html>");
  client.println();
}

void handleRequest() {
  if (header.indexOf("GET /led1/on") >= 0) {
    Serial.println("LED 1 turned ON");
    led1State = "on";
    digitalWrite(led1, HIGH);
  } 
  else if (header.indexOf("GET /led1/off") >= 0) {
    Serial.println("LED 1 turned OFF");
    led1State = "off";
    digitalWrite(led1, LOW);
  } 
  else if (header.indexOf("GET /led2/on") >= 0) {
    Serial.println("LED 2 turned ON");
    led2State = "on";
    digitalWrite(led2, HIGH);
  } 
  else if (header.indexOf("GET /led2/off") >= 0) {
    Serial.println("LED 2 turned OFF");
    led2State = "off";
    digitalWrite(led2, LOW);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Starting ESP32 Web Server...");

  // Configure pins as outputs
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);

  // Initialize outputs to LOW
  digitalWrite(led1, LOW);
  digitalWrite(led2, LOW);

  // Connect to Wi-Fi
  connectToWiFi();

  // Start web server
  server.begin();
  Serial.println("Web server started.");
}

void loop() {
  // Attempt to reconnect if Wi-Fi is lost
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi lost. Reconnecting...");
    connectToWiFi();
    delay(1000);
    return;
  }

  WiFiClient client = server.available();

  if (client) {
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New client connected.");

    String currentLine = "";
    header = "";

    while (client.connected() && (currentTime - previousTime <= timeoutTime)) {
      currentTime = millis();

      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        header += c;

        if (c == '\n') {
          if (currentLine.length() == 0) {
            handleRequest();
            sendWebPage(client);
            break;
          } else {
            currentLine = "";
          }
        } 
        else if (c != '\r') {
          currentLine += c;
        }

        previousTime = currentTime;
      }
    }

    delay(1);
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println();
  }
}