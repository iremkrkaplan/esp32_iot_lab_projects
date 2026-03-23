const int buttonPin = 4;
const int ledPin = 5;
 
// Kesme içinde değişen değişkenler için 'volatile' şarttır
volatile bool state = false;
 
// Fonksiyonu önden tanımlıyoruz (Forward Declaration) veya direkt buraya yazıyoruz
void IRAM_ATTR blink() {
  state = !state;
}
 
void setup() {
  pinMode(ledPin, OUTPUT);
  // ESP32'de buton kullanırken genelde INPUT_PULLUP daha sağlıklıdır
  pinMode(buttonPin, INPUT_PULLUP);
 
  // Fonksiyonu yukarıda tanımladığımız için artık 'blink' burada tanınacak
  attachInterrupt(digitalPinToInterrupt(buttonPin), blink, FALLING);
}
 
void loop() {
  digitalWrite(ledPin, state);
  // loop boş kalabilir veya başka işler yapabilir
}