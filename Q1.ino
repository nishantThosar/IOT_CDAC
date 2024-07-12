#include <WiFi.h>           // Wifi-Library for ESP32
#include <HTTPClient.h>     // HTTP-Client Library for GET, PUT, POST, PATCH, and DELETE Resources
#include <ArduinoJson.h>    // Arduino JSON Library
#include <esp_wifi.h>

// Set the custom MAC address in case your ESP32 is not registered with the acts network
uint8_t newMACAddress[] = {0xf4, 0x96, 0x34, 0x9d, 0xe5, 0xa4}; // f4:96:34:9d:e5:a4

// Replace with your network credentials
const char* ssid = "N";  // SSID Name to which ESP32 will connect
const char* password = "Nishant 9";  // SSID Password

// ThingSpeak server URL and write API key
const char* serverName = "https://api.thingspeak.com/channels/2597859/bulk_update.json"; // Server URL
const char* writeAPIKey = "D3QCJCZ9SFMYGR72"; //Write API key

// Timing settings
unsigned long lastTime = 0;
unsigned long timerDelay = 11000; // 11 seconds delay between sending the next data
int current = 5; //current assumption originally it will be taken by CT on the field
void setup() {
  Serial.begin(115200); // Serial Port debug message baud rate
  WiFi.mode(WIFI_STA);
  esp_wifi_set_mac(WIFI_IF_STA, &newMACAddress[0]);//mac s
  WiFi.begin(ssid, password);   // Start the Wi-Fi
  Serial.print("ESP Board MAC Address: ");
  Serial.println(WiFi.macAddress());
  Serial.println("Connecting");

  while (WiFi.status() != WL_CONNECTED) {
    delay(10000);
    Serial.println("Not Connected with Access Point ....");
  }

  // If connected, print the IP of ESP32 assigned by Router
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Timer Delay : 11 Seconds for sending data");
}

void loop() {
  if ((millis() - lastTime) > timerDelay) {
    // Generate random sensor data for 3 phases
    //Originally the data will be taken by suitable PT in the field
    int Phase_R = random(0, 250);
    int Phase_Y = random(0, 250);
    int Phase_B = random(0, 250);

    /*Power calculation
      Here assuming that power factor is unity*/
    int total_power = 3* (Phase_R* current); 
   

    // Prepare JSON document
    DynamicJsonDocument sensor_data(1024);
    sensor_data["write_api_key"] = writeAPIKey;
    JsonArray updates = sensor_data.createNestedArray("updates");
    JsonObject update = updates.createNestedObject();
    update["delta_t"] = 1;
    update["field1"] = Phase_Y;
    update["field2"] = Phase_R;
    update["field3"] = Phase_B;
    update["field4"] = total_power;

    // Serialize JSON to string and print it to the serial monitor
    String json;
    serializeJson(sensor_data, json);
    Serial.println("Serialized JSON:");
    Serial.println(json);

    // Send JSON data to ThingSpeak
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(serverName);
      http.addHeader("Content-Type", "application/json");

      int httpResponseCode = http.POST(json);
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      
      if (httpResponseCode > 0) {
        String response = http.getString();
        Serial.println("Response from server:");
        Serial.println(response);
      } else {
        Serial.print("Error on sending POST: ");
        Serial.println(httpResponseCode);
      }
      http.end();
    } else {
      Serial.println("WiFi Disconnected");
    }

    lastTime = millis();
  }
}
