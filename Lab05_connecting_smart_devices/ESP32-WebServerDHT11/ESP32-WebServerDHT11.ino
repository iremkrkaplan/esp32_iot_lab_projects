// Enhanced ESP32 DHT11 + OLED + Async Web Server Example
// - Automatic Wi-Fi reconnection
// - Clearer serial messages
// - OLED display for temperature and humidity
// - Async web server for browser monitoring

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

// Fixed Wi-Fi credentials
const char* ssid = "Erdinc's S21 FE";
const char* password = "123456789";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// OLED display settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// DHT sensor settings
#define DHTPIN 19
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// HTML page
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html {
      font-family: Arial;
      display: inline-block;
      margin: 0px auto;
      text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .dht-labels {
      font-size: 1.5rem;
      vertical-align: middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h2>ESP32 DHT Server</h2>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i>
    <span class="dht-labels">Temperature</span>
    <span id="temperature">%TEMPERATURE%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <i class="fas fa-tint" style="color:#00add6;"></i>
    <span class="dht-labels">Humidity</span>
    <span id="humidity">%HUMIDITY%</span>
    <sup class="units">&percnt;</sup>
  </p>
</body>
<script>
setInterval(function () {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 10000);

setInterval(function () {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("humidity").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/humidity", true);
  xhttp.send();
}, 10000);
</script>
</html>)rawliteral";

// Read temperature
String readDHTTemperature() {
  float t = dht.readTemperature();
  if (isnan(t)) {
    Serial.println("Failed to read temperature from DHT sensor!");
    return "--";
  }
  Serial.print("Temperature: ");
  Serial.println(t);
  return String(t);
}

// Read humidity
String readDHTHumidity() {
  float h = dht.readHumidity();
  if (isnan(h)) {
    Serial.println("Failed to read humidity from DHT sensor!");
    return "--";
  }
  Serial.print("Humidity: ");
  Serial.println(h);
  return String(h);
}

// Replace HTML placeholders
String processor(const String& var) {
  if (var == "TEMPERATURE") {
    return readDHTTemperature();
  } else if (var == "HUMIDITY") {
    return readDHTHumidity();
  }
  return String();
}

// Wi-Fi connection function
void connectToWiFi() {
  Serial.print("Connecting to Wi-Fi: ");
  Serial.println(ssid);

  WiFi.disconnect(true);
  delay(500);
  WiFi.begin(ssid, password);

  int attempts = 0;
  const int maxAttempts = 20;  // 20 x 500 ms = 10 seconds

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

// OLED update function
void updateDisplay(float t, float h) {
  display.clearDisplay();

  // Temperature
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Temperature:");
  display.setTextSize(2);
  display.setCursor(0, 10);

  if (isnan(t)) {
    display.print("--");
  } else {
    display.print(t);
  }

  display.print(" ");
  display.setTextSize(1);
  display.cp437(true);
  display.write(167);  // degree symbol
  display.setTextSize(2);
  display.print("C");

  // Humidity
  display.setTextSize(1);
  display.setCursor(0, 35);
  display.print("Humidity:");
  display.setTextSize(2);
  display.setCursor(0, 45);

  if (isnan(h)) {
    display.print("--");
  } else {
    display.print(h);
  }

  display.print(" %");
  display.display();
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Starting ESP32 DHT11 Web Server...");

  // Start DHT sensor
  dht.begin();

  // Start OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed!");
    while (true)
      ;
  }

  delay(1000);
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.display();

  // Connect to Wi-Fi
  connectToWiFi();

  // Web server routes
  server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send_P(200, "text/html", index_html, processor);
  });

  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "text/plain", readDHTTemperature());
  });

  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(200, "text/plain", readDHTHumidity());
  });

  // Start server
  server.begin();
  Serial.println("Async web server started.");
}

void loop() {
  // Attempt to reconnect if Wi-Fi is lost
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi lost. Reconnecting...");
    connectToWiFi();
    delay(1000);
    return;
  }

  // Read temperature and humidity
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (isnan(t) || isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
  } else {
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.print(" °C, Humidity: ");
    Serial.print(h);
    Serial.println(" %");
  }

  // Update OLED
  updateDisplay(t, h);

  delay(1000);
}