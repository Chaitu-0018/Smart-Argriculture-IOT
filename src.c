#include <ESP8266WiFi.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include "DHT.h"
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <ArduinoJson.h>

// WiFi Configuration
const char *ssid = "xxx";             // Enter your Wi-Fi Name
const char *pass = "xxx";          // Enter your Wi-Fi Password
WiFiClient client;

// Adafruit IO Configuration
#define MQTT_SERV "io.adafruit.com"
#define MQTT_PORT 1883
#define MQTT_NAME "xxx"              // Your Adafruit IO Username
#define MQTT_PASS "xxx" // Adafruit IO AIO key

// Weather API Configuration
const char server[] = "api.openweathermap.org";
String nameOfCity = "Chennai,IN";
String apiKey = "09f88896a6ee88c86a7a9d1a7512c224";
String text;
const char* icon = "";

// JSON Parsing Variables
int jsonend = 0;
boolean startJson = false;
int status = WL_IDLE_STATUS;
#define JSON_BUFF_DIMENSION 2500

// Timing Variables
unsigned long lastConnectionTime = 10 * 60 * 1000;  // Last time you connected to the server, in milliseconds
const unsigned long postInterval = 10 * 60 * 1000;  // Posting interval of 10 minutes
const unsigned long Interval = 50000;               // Sensor reading interval
unsigned long previousTime = 0;

// Pin Definitions
const int ldrPin = D1;         // LDR sensor pin
const int ledPin = D0;         // LED pin
const int moisturePin = A0;    // Moisture sensor pin
const int motorPin = D8;       // Water pump pin
#define ONE_WIRE_BUS 4         // D2 pin of NodeMCU for DS18B20
#define DHTTYPE DHT11          // DHT sensor type
#define dht_dpin D4            // DHT sensor pin

// Sensor Variables
float moisturePercentage;      // Moisture reading
int temperature, humidity, soiltemp;

// Initialize Sensors
DHT dht(dht_dpin, DHTTYPE);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// Set up MQTT client
Adafruit_MQTT_Client mqtt(&client, MQTT_SERV, MQTT_PORT, MQTT_NAME, MQTT_PASS);

// Set up the feed you're publishing to
Adafruit_MQTT_Publish Moisture = Adafruit_MQTT_Publish(&mqtt, MQTT_NAME "/f/Moisture");  // Moisture is the feed name
Adafruit_MQTT_Publish Temperature = Adafruit_MQTT_Publish(&mqtt, MQTT_NAME "/f/Temperature");
Adafruit_MQTT_Publish Humidity = Adafruit_MQTT_Publish(&mqtt, MQTT_NAME "/f/Humidity");
Adafruit_MQTT_Publish SoilTemp = Adafruit_MQTT_Publish(&mqtt, MQTT_NAME "/f/SoilTemp");
Adafruit_MQTT_Publish WeatherData = Adafruit_MQTT_Publish(&mqtt, MQTT_NAME "/f/WeatherData");

// Set up the feed you're subscribing to
Adafruit_MQTT_Subscribe LED = Adafruit_MQTT_Subscribe(&mqtt, MQTT_NAME "/f/LED");
Adafruit_MQTT_Subscribe Pump = Adafruit_MQTT_Subscribe(&mqtt, MQTT_NAME "/f/Pump");

void setup() {
  Serial.begin(115200);
  delay(10);
  
  // Initialize sensors
  dht.begin();
  sensors.begin();
  
  // Set up MQTT subscriptions
  mqtt.subscribe(&LED);
  mqtt.subscribe(&Pump);
  
  // Configure pins
  pinMode(motorPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(ldrPin, INPUT);
  
  // Initial state
  digitalWrite(motorPin, LOW);  // Keep motor off initially
  digitalWrite(ledPin, HIGH);   // Turn LED on initially
  
  // Reserve memory for JSON
  text.reserve(JSON_BUFF_DIMENSION);
  
  // Connect to WiFi
  Serial.println("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");  // Print ... till not connected
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
}

void loop() {
  unsigned long currentTime = millis();
  
  // Ensure MQTT connection
  MQTT_connect();
  
  // Check if it's time to make an HTTP request for weather data
  if (millis() - lastConnectionTime > postInterval) {
    // Note the time that the connection was made
    lastConnectionTime = millis();
    makehttpRequest();
  }
  
  // LDR (Light) Sensor Logic
  int ldrStatus = analogRead(ldrPin);
  if (ldrStatus <= 200) {
    digitalWrite(ledPin, HIGH);
    Serial.print("Its DARK, Turn on the LED : ");
    Serial.println(ldrStatus);
  } else {
    digitalWrite(ledPin, LOW);
    Serial.print("Its BRIGHT, Turn off the LED : ");
    Serial.println(ldrStatus);
  }
  
  // Soil Moisture Sensor Logic
  moisturePercentage = (100.00 - ((analogRead(moisturePin) / 1023.00) * 100.00));
  Serial.print("Soil Moisture is = ");
  Serial.print(moisturePercentage);
  Serial.println("%");
  
  // Automatic irrigation control
  if (moisturePercentage < 35) {
    digitalWrite(motorPin, HIGH);  // Turn on motor
  }
  
  if (moisturePercentage > 38) {
    digitalWrite(motorPin, LOW);   // Turn off motor
  }
  
  // Read temperature and humidity
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println();
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.println();
  
  // Read soil temperature
  sensors.requestTemperatures();
  soiltemp = sensors.getTempCByIndex(0);
  Serial.println("Soil Temperature: ");
  Serial.println(soiltemp);
  
  // Publish data to Adafruit IO at specific intervals
  if (currentTime - previousTime >= Interval) {
    if (!Temperature.publish(temperature)) {
      // Publishing failed
    }
    
    if (!Humidity.publish(humidity)) {
      // Publishing failed
      //delay(30000);
    }
    
    if (!SoilTemp.publish(soiltemp)) {
      // Publishing failed
    }
    
    if (!WeatherData.publish(icon)) {
      // Publishing failed
    }
    
    previousTime = currentTime;
  }
  
  // Check for subscription messages
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(5000))) { // Don't use this until you are controlling something
    if (subscription == &LED) {
      // Print the new value to the serial monitor
      Serial.println((char*)LED.lastread);
      
      if (!strcmp((char*)LED.lastread, "OFF")) {
        digitalWrite(ledPin, LOW);
      }
      
      if (!strcmp((char*)LED.lastread, "ON")) {
        digitalWrite(ledPin, HIGH);
      }
    }
    
    if (subscription == &Pump) {
      // Print the new value to the serial monitor
      Serial.println((char*)Pump.lastread);
      
      if (!strcmp((char*)Pump.lastread, "OFF")) {
        digitalWrite(motorPin, HIGH);
      }
      
      if (!strcmp((char*)Pump.lastread, "ON")) {
        digitalWrite(motorPin, LOW);
      }
    }
  }
  
  delay(9000);
  // client.publish(WeatherData, icon)
}

// Function to connect and reconnect to MQTT as needed
void MQTT_connect() {
  int8_t ret;
  
  // Stop if already connected
  if (mqtt.connected()) {
    return;
  }
  
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // Connect will return 0 for connected
    mqtt.disconnect();
    delay(5000);  // Wait 5 seconds
    retries--;
    if (retries == 0) {
      // Basically die and wait for WDT to reset
      while (1);
    }
  }
}

// Function to make HTTP request for weather data
void makehttpRequest() {
  // Close any connection before sending a new request
  client.stop();
  
  char c = 0;
  while (client.available()) {
    c = client.read();
    // Since JSON contains equal number of open and close curly brackets,
    // we can determine when a JSON is completely received by counting them
    //Serial.print(c);
    if (c == '{') {
      startJson = true; // Set startJson true to indicate JSON message has started
      jsonend++;
    }
    if (c == '}') {
      jsonend--;
    }
    if (startJson == true) {
      text += c;
    }
    // If jsonend = 0 then we have received equal number of curly braces
    if (jsonend == 0 && startJson == true) {
      // Parse JSON here (function call is incomplete in the original code)
    }
  }
}

// Function to parse JSON data
void parseJson(const char* jsonString) {
  //StaticJsonBuffer<4000> jsonBuffer;
  const size_t bufferSize = 2*JSON_ARRAY_SIZE(1) + JSON_ARRAY_SIZE(2) + 
                           4*JSON_OBJECT_SIZE(1) + 3*JSON_OBJECT_SIZE(2) +
                           3*JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) +
                           2*JSON_OBJECT_SIZE(7) + 2*JSON_OBJECT_SIZE(8) + 720;
  DynamicJsonDocument doc(1024);
  
  DeserializationError error = deserializeJson(doc, jsonString);
  if (error) {
    Serial.print(" Deserialization failed");
    return;
  }
  
  JsonArray list = doc["list"];
  JsonObject nowT = list[0];
  JsonObject later = list[1];
  JsonObject tommorow = list[2];
  
  // String conditions = list.weather.main;
  // Including temperature and humidity for those who may wish to hack it in
  String city = doc["city"]["name"];
  String weatherNow = nowT["weather"][0]["description"];
  String weatherLater = later["weather"][0]["description"];
  String list12 = later["weather"][0]["list"];
}
