// FOR USE IN ARDUINO IDE
#include <SD.h>
#include <SPI.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>

#define CS_PIN 5  // Chip Select pin for the SD card

AsyncWebServer server(80);  // Create AsyncWebServer object on port 80

void setup() {
  Serial.begin(115200);

  if (!SD.begin(CS_PIN)) {
    Serial.println("Card Mount Failed");
    return;
  }

  // Serve the file explorer web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SD, "/index.html", "text/html");
  });

  // Endpoint to list files
  server.on("/list", HTTP_GET, [](AsyncWebServerRequest *request) {
    String fileList = listFiles(SD.open("/"));
    request->send(200, "text/plain", fileList);
  });

  // Endpoint to upload files
  server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request) {
    request->send(200);
  }, onFileUpload);

  // Endpoint to download files
  server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("file")) {
      String filename = request->getParam("file")->value();
      request->send(SD, filename, "application/octet-stream");
    } else {
      request->send(400, "text/plain", "File Not Specified");
    }
  });

  // Endpoint to delete files
  server.on("/delete", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("file")) {
      String filename = request->getParam("file")->value();
      if (SD.remove(filename)) {
        request->send(200, "text/plain", "File Deleted");
      } else {
        request->send(400, "text/plain", "Failed to Delete File");
      }
    } else {
      request->send(400, "text/plain", "File Not Specified");
    }
  });

  server.begin();  // Start server
}

void loop() {
  // Nothing needed here, the web server is asynchronous
}

String listFiles(File dir) {
  String output = "";
  while (true) {
    File entry = dir.openNextFile();
    if (!entry) {
      break;
    }
    output += entry.name();
    if (entry.isDirectory()) {
      output += "/\n";
    } else {
      output += " (" + String(entry.size()) + " bytes)\n";
    }
    entry.close();
  }
  return output;
}

void onFileUpload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  if (!index) {
    // Open file for writing
    Serial.printf("Upload Start: %s\n", filename.c_str());
    request->_tempFile = SD.open("/" + filename, FILE_WRITE);
  }
  if (len) {
    // Write data to file
    request->_tempFile.write(data, len);
  }
  if (final) {
    // Close file when done
    request->_tempFile.close();
    Serial.printf("Upload Complete: %s\n", filename.c_str());
  }
}
