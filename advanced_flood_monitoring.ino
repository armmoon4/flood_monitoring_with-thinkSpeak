#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <ThingSpeak.h>
#include <Wire.h>

SoftwareSerial mySerial(D6, D7);
LiquidCrystal_I2C lcd(0x27, 16, 2);

int FloatSensor = D5;
int buttonState = 1;
const int trigPin1 = D3;
const int echoPin1 = D4;
unsigned long startMillis;
unsigned long currentMillis;
const unsigned long period = 10000;
long duration1;
int distance1;

// LED Pins
const int redLED = D0;   
const int greenLED = D8; 

// WiFi credentials and ThingSpeak API key
const char* ssid = "Gunslinger"; // Replace with your WiFi SSID
const char* password = "12@#hridoy#@12"; // Replace with your WiFi Password
const char* server = "api.thingspeak.com";
const String writeAPIKey = "yourapikey";

WiFiClient client;

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
  Wire.begin(D2, D1);
  pinMode(FloatSensor, INPUT_PULLUP);

  // Initialize LEDs as outputs
  pinMode(redLED, OUTPUT);
  pinMode(greenLED, OUTPUT);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("   WELCOME TO");
  lcd.setCursor(0, 1);
  lcd.print("BYTE_BROS");
  delay(2000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" SMART");
  lcd.setCursor(0, 1);
  lcd.print(" FLOOD MONITORING");
  delay(3000);
  lcd.clear();
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    lcd.print(".");
  }
  Serial.println("WiFi connected");
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Connected");
  delay(2000);
}

void loop() {
  ultersonic();

  if (buttonState == HIGH) {
    Serial.println("WATER LEVEL -HIGH");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("  HIGH ALERTS");
    lcd.setCursor(0, 1);
    lcd.print("WATER LEVEL-");
    lcd.setCursor(13, 1);
    lcd.print(distance1);

    // Turn on red LED for high alert, turn off green LED
    digitalWrite(redLED, HIGH);
    digitalWrite(greenLED, LOW);

  } else {
    Serial.println("WATER LEVEL - LOW");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("   LOW ALERTS");
    lcd.setCursor(0, 1);
    lcd.print("WATER LEVEL-");
    lcd.setCursor(13, 1);
    lcd.print(distance1);

    // Turn on green LED for low alert, turn off red LED
    digitalWrite(greenLED, HIGH);
    digitalWrite(redLED, LOW);
  }

  currentMillis = millis();
  if (currentMillis - startMillis >= period) {
    startMillis = currentMillis;
    sendDataToThingSpeak();
  }
}

void ultersonic() {
  digitalWrite(trigPin1, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin1, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin1, LOW);
  duration1 = pulseIn(echoPin1, HIGH);
  distance1 = duration1 * 0.207 / 2;
  Serial.println(distance1);
  buttonState = digitalRead(FloatSensor);
  delay(100);
}

void sendDataToThingSpeak() {
  if (client.connect(server, 80)) {
    // Formulate the request string
    String postStr = "api_key=" + writeAPIKey + 
                     "&field1=" + String(distance1) + 
                     "&field2=" + String(buttonState);

    // Send HTTP request
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);

    Serial.println("Data sent to ThingSpeak: " + postStr);
    client.stop(); // Close the connection
  } else {
    Serial.println("Connection to ThingSpeak failed.");
  }
}
