#include <SPI.h>
#include <LoRa.h>
#include <esp_sleep.h>
#include <driver/adc.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include "OTAHandler.h"

#define ss 18
#define rst 14
#define dio0 26
#define wakePin 2
#define buttonPin 15 

#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for microseconds to seconds */
#define HOURLY_SLEEP_TIME  3600
#define NIGHTLY_SLEEP_TIME 25200  // 7 hours (9 PM - 7 AM)
#define DAILY_SYNC_INTERVAL 86400  // 24 hours in seconds

RTC_DATA_ATTR int counter = 0;
RTC_DATA_ATTR int checkinCounter = 0;
RTC_DATA_ATTR bool timeInitialized = false;
RTC_DATA_ATTR bool eventProcessedThisCycle = false;
RTC_DATA_ATTR time_t lastSyncTime = 0; 

const int maxRetries = 5;     
const int ackTimeout = 5000;     // Time to wait for an ACK (milliseconds)
const int timeRequestTimeout = 10000;  // 10 seconds timeout for time request

void setup() {
  Serial.begin(115200);
  while (!Serial);

  pinMode(buttonPin, INPUT_PULLUP); 
  pinMode(ledPin, OUTPUT);          

  // Log the state of the button on boot
  int buttonState = digitalRead(buttonPin);
  Serial.print("Button state on boot: ");
  Serial.println(buttonState == HIGH ? "Not Pressed (HIGH)" : "Pressed (LOW)");

  // Check if the device woke up from an external interrupt (button press)
  if (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT0 && buttonState == LOW) {
    startOTAMode(); 
  }

  if (!initializeLoRa()) {
    Serial.println("LoRa initialization failed!");
    return;
  }

  // Check if time synchronization is required
  if (!timeInitialized || needsTimeSync()) {
    if (requestTimeFromReceiver()) {
      timeInitialized = true;
      lastSyncTime = time(NULL);  // Record the time of the last successful sync
    }
  }

  configureWakeUpSources();
}

void loop() {
  delay(100); // Small delay to stabilize readings

  // Hatch opening...
  bool wokeUpByEvent = (esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_EXT1);

  eventProcessedThisCycle = false;

  // Check if it's night time and no event triggered the wake-up
  bool isNight = isNightTime();
  if (!wokeUpByEvent && isNight) {
    sleepUntilMorning(); 
    return;             
  }

  // Construct the message based on whether it was an event or a regular check-in
  String message = constructMessage(wokeUpByEvent);

  // Attempt to send the LoRa message and wait for an ACK
  if (sendLoRaMessage(message)) {
    Serial.println("Message sent and ACK received.");
  } else {
    Serial.println("Failed to receive ACK after max retries.");
  }

  prepareForDeepSleep();
}

bool initializeLoRa() {
  LoRa.setPins(ss, rst, dio0);
  LoRa.setTxPower(19); 

  if (!LoRa.begin(915E6)) {
    return false;
  }

  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initializing OK!");
  return true;
}

bool isNightTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return false; // Fail-safe: consider it not night if time can't be obtained
  }

  int currentHour = timeinfo.tm_hour;
  return (currentHour >= 21 || currentHour < 7); // Consider 9 PM to 7 AM as night time
}

void sleepUntilMorning() {
  Serial.println("Night time detected. Sleeping until morning...");

  // Calculate seconds until 7 AM
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time. Sleeping for default duration.");
    esp_sleep_enable_timer_wakeup(HOURLY_SLEEP_TIME * uS_TO_S_FACTOR);
    esp_deep_sleep_start();
    return;
  }

  int currentHour = timeinfo.tm_hour;
  int secondsUntil7AM;
  if (currentHour >= 7 && currentHour < 21) {
    // It's day time, should not reach here
    secondsUntil7AM = HOURLY_SLEEP_TIME;
  } else {
    // It's night time, calculate time until 7 AM
    int hoursUntil7AM = (7 - currentHour + 24) % 24;
    int minutesUntil7AM = (60 - timeinfo.tm_min) % 60;
    int secondsUntil7AMPart = (60 - timeinfo.tm_sec) % 60;
    secondsUntil7AM = hoursUntil7AM * 3600 + minutesUntil7AM * 60 + secondsUntil7AMPart;
  }

  uint64_t sleepDuration = (uint64_t)secondsUntil7AM * uS_TO_S_FACTOR;

  esp_sleep_enable_timer_wakeup(sleepDuration);
  Serial.printf("Sleeping for %d seconds until 7 AM\n", secondsUntil7AM);
  esp_deep_sleep_start();
}

String constructMessage(bool wokeUpByEvent) {
  String message;
  if (wokeUpByEvent && !eventProcessedThisCycle) {
    counter++;
    eventProcessedThisCycle = true;
    message = "EVENT " + String(counter);
    Serial.println("Constructed EVENT message");
  } else if (wokeUpByEvent && eventProcessedThisCycle) {
    message = "EVENT " + String(counter);  // Use the same counter value
    Serial.println("Constructed repeat EVENT message");
  } else {
    checkinCounter++;
    message = "CHECKIN " + String(checkinCounter);
    Serial.println("Constructed CHECKIN message");
  }
  return message;
}

bool sendLoRaMessage(String message) {
  bool ackReceived = false;
  for (int i = 0; i < maxRetries && !ackReceived; i++) {
    LoRa.beginPacket();
    LoRa.print(message);
    LoRa.endPacket();
    Serial.print("Sent: ");
    Serial.println(message);

    // Wait for ACK
    long startTime = millis();
    while (millis() - startTime < ackTimeout) {
      int packetSize = LoRa.parsePacket();
      if (packetSize) {
        String ackMessage = "";
        while (LoRa.available()) {
          ackMessage += (char)LoRa.read();
        }
        Serial.print("Received: ");
        Serial.println(ackMessage);
        if (ackMessage == "ACK") {
          ackReceived = true;
          break;
        }
      }
    }

    if (!ackReceived) {
      Serial.println("ACK not received, retrying...");
    }
  }

  return ackReceived;
}

bool needsTimeSync() {
  time_t currentTime = time(NULL);

  if (lastSyncTime == 0 || (currentTime - lastSyncTime) >= DAILY_SYNC_INTERVAL) {
    return true;
  }
  return false;
}

void prepareForDeepSleep() {
  // Calculate if daily sync is needed before sleeping
  time_t currentTime = time(NULL);

  if ((currentTime - lastSyncTime) >= DAILY_SYNC_INTERVAL) {
    // Time to resync
    if (requestTimeFromReceiver()) {
      lastSyncTime = currentTime;
    }
  }

  esp_deep_sleep_start();
}

bool requestTimeFromReceiver() {
  Serial.println("Requesting time from receiver");

  LoRa.beginPacket();
  LoRa.print("TIME_REQUEST");
  LoRa.endPacket();

  long startTime = millis();
  while (millis() - startTime < timeRequestTimeout) {
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
      String timeMessage = "";
      while (LoRa.available()) {
        timeMessage += (char)LoRa.read();
      }
      if (timeMessage.startsWith("TIME ")) {
        setTimeFromMessage(timeMessage);
        return true;
      }
    }
  }

  Serial.println("Failed to get time from receiver");
  return false;
}

void setTimeFromMessage(String timeMessage) {
  int year, month, day, hour, minute, second;
  sscanf(timeMessage.c_str(), "TIME %d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);

  struct tm timeinfo;
  timeinfo.tm_year = year - 1900;
  timeinfo.tm_mon = month - 1;
  timeinfo.tm_mday = day;
  timeinfo.tm_hour = hour;
  timeinfo.tm_min = minute;
  timeinfo.tm_sec = second;
  timeinfo.tm_isdst = 0;

  time_t t = mktime(&timeinfo);
  struct timeval now_tv = { .tv_sec = t, .tv_usec = 0 };
  settimeofday(&now_tv, NULL);

  Serial.printf("Time set to: %04d-%02d-%02d %02d:%02d:%02d\n", 
                year, month, day, hour, minute, second);
}

void configureWakeUpSources() {
  pinMode(wakePin, INPUT_PULLUP); 
  pinMode(buttonPin, INPUT_PULLUP); 
  esp_sleep_enable_ext0_wakeup(static_cast<gpio_num_t>(buttonPin), LOW);
  esp_sleep_enable_ext1_wakeup(BIT(wakePin), ESP_EXT1_WAKEUP_ALL_LOW); 
  esp_sleep_enable_timer_wakeup(HOURLY_SLEEP_TIME * uS_TO_S_FACTOR);
}