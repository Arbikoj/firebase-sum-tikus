
#include <Arduino.h>
#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

#include <NTPClient.h>
#include <WiFiUdp.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "Masjid keputih"
#define WIFI_PASSWORD "Masjid354"

// Insert Firebase project API Key
#define API_KEY secrett"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "secrett"

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0, countData = 0, countDataSize = 0, tmpMonth = 0;
int currentData;
bool signupOK = false;

String idDevice = "id354";

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

//Week Days
String weekDays[7] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };

//Month names
String months[12] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };


void setup() {
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
    signupOK = true;
  } else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback;  //see addons/TokenHelper.h

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Initialize a NTPClient to get time
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  timeClient.setTimeOffset(25200);
}

void loop() {

  timeClient.update();

  time_t epochTime = timeClient.getEpochTime();
  struct tm *ptm = gmtime((time_t *)&epochTime);

  String formattedTime = timeClient.getFormattedTime();

  // TAHUN
  int currentYear = ptm->tm_year + 1900;
  // BULAN
  int currentMonth = ptm->tm_mon + 1;
  // TGL
  int monthDay = ptm->tm_mday;




  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 50 || sendDataPrevMillis == 0)) {

    sendDataPrevMillis = millis();

    // Count Data
    if (Firebase.RTDB.getArray(&fbdo, String(currentYear) + "/" + String(currentMonth) + "/" + String(monthDay) + "/value/")) {
      // int size = sizeof(fbdo.to<FirebaseJsonArray>().raw());
      FirebaseJsonArray array = fbdo.to<FirebaseJsonArray>().raw();
      countDataSize = array.size();
      Serial.println(countDataSize);
      Serial.println(fbdo.to<FirebaseJsonArray>().raw());
      // Serial.println(array.size());
      // Serial.println("sz: "+size);
      // update data
      // Firebase.RTDB.setFloat(&fbdo, String(currentYear)+ "/" + String(currentMonth) + "/" + String(monthDay) + "/total", size);
    } else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }

    if (countDataSize > 0) {
      countData = countDataSize;
    } else if (tmpMonth != ptm->tm_mday) {
      countData = 0;
      // tmpMonth = ptm->tm_mday;
    }

    while (Serial.available() > 0) {
      char inByte = Serial.read();
      if (inByte == 49) {

        // Data
        if (Firebase.RTDB.setString(&fbdo, String(currentYear) + "/" + String(currentMonth) + "/" + String(monthDay) + "/value/" + countData, formattedTime)) {
          // update data
          Firebase.RTDB.setFloat(&fbdo, String(currentYear) + "/" + String(currentMonth) + "/" + String(monthDay) + "/total", countDataSize + 1);
        } else {
          Serial.println("FAILED");
          Serial.println("REASON: " + fbdo.errorReason());
        }

        // GET DATA
        //     if (Firebase.RTDB.getInt(&fbdo, String(currentYear)+ "/" + String(currentMonth) + "/" + String(monthDay) + "/total"))
        // {
        //   currentData = fbdo.intData();
        //   // Serial.println(currentData);
        // } else {
        //   Serial.print("Error in getString, ");
        //   Serial.println(fbdo.errorReason());
        // }


        // FirebaseJson json;
        // json.setDoubleDigits(3);
        //         json.add("value", count);

        //         Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, "/test/json", &json) ? "ok" : fbdo.errorReason().c_str());

        // Serial.printf("Get json... %s\n", Firebase.RTDB.getJSON(&fbdo, "/test/json") ? fbdo.to<FirebaseJson>().raw() : fbdo.errorReason().c_str());

        //     FirebaseJson jVal;
        //         Serial.printf("Get json ref... %s\n", Firebase.RTDB.getJSON(&fbdo, "/test/json", &jVal) ? jVal.raw() : fbdo.errorReason().c_str());

        // FirebaseJsonArray arr;
        //         arr.setFloatDigits(2);
        //         arr.setDoubleDigits(4);
        //         arr.add("a", "b", "c", true, 45, (float)6.1432, 123.45692789);

        //         Serial.printf("Set array... %s\n", Firebase.RTDB.setArray(&fbdo, "/test/array", &arr) ? "ok" : fbdo.errorReason().c_str());

        // Serial.printf("Get array... %s\n", Firebase.RTDB.getArray(&fbdo, "/2023/10/1/value") ? fbdo.to<FirebaseJsonArray>().raw() : fbdo.errorReason().c_str());

        //         Serial.printf("Push json... %s\n", Firebase.RTDB.pushJSON(&fbdo, "/test/push", &json) ? "ok" : fbdo.errorReason().c_str());

        //         json.set("value", count + 0.29745);
        //         Serial.printf("Update json... %s\n\n", Firebase.RTDB.updateNode(&fbdo, "/test/push/" + fbdo.pushName(), &json) ? "ok" : fbdo.errorReason().c_str());
        tmpMonth = monthDay;

        countData++;
        count++;
      }
    }
  }
}
