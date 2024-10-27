// FOR USE IN ARDUINO IDE
// TO CUSTOMISE WHAT THE SAVED FILE'S NAME WILL BE, EDIT LINE 69
#include <WiFi.h>
#include <HTTPClient.h>
#include <SD.h>
#include <SPI.h>

#define CS_PIN 5  // SD Card Chip Select pin

// Wi-Fi credentials
const char* ssid = "YOUR_SSID";      // Replace with your Wi-Fi SSID
const char* password = "YOUR_PASSWORD";  // Replace with your Wi-Fi password

void setup() {
  Serial.begin(115200);

  // Connect to Wi-Fi
  connectToWiFi();

  // Initialize SD card
  if (!SD.begin(CS_PIN)) {
    Serial.println("SD Card Mount Failed");
    return;
  }
  Serial.println("SD Card initialized successfully");

  Serial.println("Please paste the URL of the file you want to download:");
}

void loop() {
  // Check if there's input from the Serial Monitor
  if (Serial.available()) {
    String url = Serial.readStringUntil('\n');
    url.trim();  // Remove any trailing spaces or newline characters

    if (url.length() > 0) {
      downloadFile(url);
    }
  }
}

void connectToWiFi() {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi...");
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  
  Serial.println("\nConnected to Wi-Fi");
}

void downloadFile(String url) {
  HTTPClient http;
  Serial.print("Downloading file from: ");
  Serial.println(url);

  http.begin(url);  // Initialize the HTTP request
  
  int httpCode = http.GET();  // Make the request

  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      // Get the file content from the server
      WiFiClient *stream = http.getStreamPtr();

      // Open the file on the SD card for writing
      File file = SD.open("/index.html", FILE_WRITE);
      if (!file) {
        Serial.println("Failed to open file on SD card for writing");
        return;
      }

      // Read the file from the HTTP response and write it to the SD card
      uint8_t buffer[128] = {0};
      int len = 0;
      while (http.connected() && (len = stream->available())) {
        if (len > 128) {
          len = 128;
        }
        int bytesRead = stream->readBytes(buffer, len);
        file.write(buffer, bytesRead);
      }

      Serial.println("File successfully downloaded to SD card as /index.html");
      file.close();
    } else {
      Serial.printf("Failed to download file, HTTP error: %d\n", httpCode);
    }
  } else {
    Serial.printf("HTTP request failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();  // End the HTTP request
}
