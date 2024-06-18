#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <ESP8266WiFi.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Pin Definitions
#define DHTPIN 2          // Pin where the DHT sensor is connected
#define PIRPIN 3          // Pin where the PIR sensor is connected
#define REDPIN 5          // Red channel of LED strip
#define GREENPIN 6        // Green channel of LED strip
#define BLUEPIN 9         // Blue channel of LED strip
#define OLED_RESET 4      // Reset pin for OLED (usually not needed)
#define WIFI_TX 10        // ESP8266 TX pin
#define WIFI_RX 11        // ESP8266 RX pin

// DHT Sensor
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// OLED Display
Adafruit_SSD1306 display(OLED_RESET);

// WiFi credentials
const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";

// Variables
float temperature = 0.0;
float humidity = 0.0;
bool motionDetected = false;

void setup() {
  // Initialize Serial
  Serial.begin(115200);
  
  // Initialize DHT22
  dht.begin();
  
  // Initialize PIR sensor
  pinMode(PIRPIN, INPUT);
  
  // Initialize LED strip
  pinMode(REDPIN, OUTPUT);
  pinMode(GREENPIN, OUTPUT);
  pinMode(BLUEPIN, OUTPUT);

  // Initialize OLED Display
  display.begin(SSD1306_I2C_ADDRESS, OLED_RESET);
  display.clearDisplay();
  display.display();

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
}

void loop() {
  // Read DHT22 Sensor
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();

  // Check if any reads failed and exit early
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  
  // Read PIR Sensor
  motionDetected = digitalRead(PIRPIN) == HIGH;

  // Print to Serial Monitor
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" °C, Humidity: ");
  Serial.print(humidity);
  Serial.print(" %, Motion: ");
  Serial.println(motionDetected ? "Detected" : "Not Detected");

  // Update OLED Display
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.print("Temp: "); display.print(temperature); display.println(" C");
  display.print("Hum: "); display.print(humidity); display.println(" %");
  display.print("Motion: "); display.println(motionDetected ? "Yes" : "No");
  display.display();

  // Control LED Strip based on motion
  if (motionDetected) {
    setColor(255, 0, 0);  // Red for motion detected
  } else {
    setColor(0, 0, 255);  // Blue for no motion
  }

  // Send data to server
  sendDataToServer(temperature, humidity, motionDetected);

  delay(2000);  // Wait for 2 seconds
}

void setColor(int red, int green, int blue) {
  analogWrite(REDPIN, red);
  analogWrite(GREENPIN, green);
  analogWrite(BLUEPIN, blue);
}

void sendDataToServer(float temp, float hum, bool motion) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    if (client.connect("your_server_address", 80)) {
      client.print("GET /update?temp=");
      client.print(temp);
      client.print("&hum=");
      client.print(hum);
      client.print("&motion=");
      client.print(motion ? "1" : "0");
      client.println(" HTTP/1.1");
      client.println("Host: your_server_address");
      client.println("Connection: close");
      client.println();
    }
    client.stop();
  }
}
