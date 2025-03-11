# 🌱 Smart Agriculture IoT Monitoring System  

This project is a **Smart Agriculture IoT Monitoring System** using an **ESP32**, **DHT11 sensor**, **soil moisture sensor**, and **humidity sensor**. It utilizes **IoT protocols** and **Adafruit libraries** to monitor environmental conditions and display real-time data.  

## 📌 Features  
✔️ Collects real-time data from DHT11, soil moisture, and humidity sensors.  
✔️ Sends data to **ThingSpeak** or **Adafruit Cloud** using API keys.  
✔️ Displays sensor values graphically on a web dashboard.  
✔️ Provides a **real-time weather report** for the area.  

# 🔧 Hardware Requirements  
- **ESP32** microcontroller  
- **DHT11** temperature & humidity sensor  
- **Soil moisture sensor**  
- **Jumper wires**  
- **Power source (5V)**  

## 🛠️ Setup Instructions  

### Step 1: Connect the Sensors  
Follow the block diagram to wire the **ESP32** with:  
- **DHT11** → Data pin to ESP32 GPIO  
- **Soil Moisture Sensor** → Analog pin to ESP32 GPIO  

### Step 2: Install Arduino IDE  
- Download & install **Arduino IDE** from [here](https://www.arduino.cc/en/software).

### Step 3: Add ESP32 Board to Arduino IDE  
1. Open **Arduino IDE**  
2. Go to **File → Preferences** (Shortcut: `Ctrl + ,`)  
3. Copy and paste this URL into **Additional Board Manager URLs**:
4. 4. Click **OK**, then install **ESP32 Board** from the Board Manager.  

### Step 4: Install Required Libraries  
Install the following libraries from the **Library Manager**:  
- **ESP8266WiFi**  
- **DallasTemperature**  
- **OneWire**  
- **DHT**  
- **Adafruit_MQTT**  
- **Adafruit_MQTT_Client**  

⚠️ Make sure to install them from trusted sources like **EspressIF** and **Adafruit**.  

### Step 5: Get API Key for Cloud Platform  
- Sign up on **[ThingSpeak](https://thingspeak.com/)** or **[Adafruit IO](https://io.adafruit.com/)**.  
- Generate an **API key** to send data from ESP32.  

### Step 6: Upload the Code  
- Open the provided **Arduino sketch**.  
- Enter your **Wi-Fi credentials** and **API key**.  
- Upload the code to the **ESP32**.  

## 📊 Output  
- Sensor data will be displayed graphically on **ThingSpeak** or **Adafruit Cloud**.  
- Includes a **real-time weather report** of your area.  

