#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 
#define SCREEN_HEIGHT 64 
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const int trigPin = 5;
const int echoPin = 18;
const int pinR = 27;
const int pinG = 26;
const int pinB = 25;

#define SOUND_SPEED 0.034
float distanceCm;

unsigned long previousBlinkMillis = 0;
unsigned long previousSensorMillis = 0;
bool ledState = false;
long blinkInterval = 500; 
int rangeMode = 0; // 1=Red, 2=Yellow, 3=Green

void setup() {
  Serial.begin(115200);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(pinR, OUTPUT);
  pinMode(pinG, OUTPUT);
  pinMode(pinB, OUTPUT);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    for(;;); 
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
}

void loop() {
  unsigned long currentMillis = millis();

  // Task 1: Sensor & Range Logic (Every 100ms for responsiveness)
  if (currentMillis - previousSensorMillis >= 100) {
    previousSensorMillis = currentMillis;

    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    
    long duration = pulseIn(echoPin, HIGH);
    distanceCm = duration * SOUND_SPEED / 2;

    // --- RANGE LOGIC ---
    if (distanceCm > 0 && distanceCm <= 15) {
      rangeMode = 1;      // RED
      blinkInterval = 100; // Rapid blink
    } 
    else if (distanceCm > 15 && distanceCm < 50) {
      rangeMode = 2;      // YELLOW (Red + Green)
      blinkInterval = 300; // Medium blink
    } 
    else if (distanceCm >= 50) {
      rangeMode = 3;      // GREEN
      blinkInterval = 800; // Slow blink
    } else {
      rangeMode = 0;      // Out of range / Error
    }

    // Update OLED
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.println("DISTANCE:");
    display.setTextSize(3);
    display.print((int)distanceCm);
    display.setTextSize(1);
    display.println(" cm");
    display.display();
  }

  // Task 2: Non-Blocking Blink Logic
  if (currentMillis - previousBlinkMillis >= blinkInterval) {
    previousBlinkMillis = currentMillis;
    ledState = !ledState; // Toggle state

    // Reset all pins first
    digitalWrite(pinR, LOW);
    digitalWrite(pinG, LOW);
    digitalWrite(pinB, LOW);

    if (ledState) {
      if (rangeMode == 1) { // RED
        digitalWrite(pinR, HIGH);
      } 
      else if (rangeMode == 2) { // YELLOW
        digitalWrite(pinR, HIGH);
        digitalWrite(pinG, HIGH);
      } 
      else if (rangeMode == 3) { // GREEN
        digitalWrite(pinG, HIGH);
      }
    }
  }
}