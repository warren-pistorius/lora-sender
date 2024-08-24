#ifndef OTA_HANDLER_H
#define OTA_HANDLER_H

#include <WiFi.h>
#include <ArduinoOTA.h>

#define ledPin 25  // Define the pin connected to the LED

void startOTAMode() {
  Serial.println("Entering OTA mode...");
  digitalWrite(ledPin, HIGH); // Turn on LED

  // Set up OTA
  WiFi.begin("Addie", "123xxx"); 
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("IP Address is: ");
  Serial.println(WiFi.localIP());

  ArduinoOTA.setHostname("LoRa32-T3-Sender");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }
    Serial.println("Start updating " + type);
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });

  ArduinoOTA.begin();
  Serial.println("OTA Ready. Connect to your device's IP address for updates.");

  while (true) {
    ArduinoOTA.handle(); // Keep OTA mode active
  }
}

#endif
