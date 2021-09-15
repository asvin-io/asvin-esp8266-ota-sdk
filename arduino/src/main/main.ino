/*
 * Asvin Esp8266 HTTP OTA Demo
 *
 * The sketch was developed for a prototype consist of Nodemcu ESP8266 board
 * . The board connects to asvin platform and perform OTA
 * firmware updates.
 *
 * Written by Rohit Bohara, Apache-2.0 License
 */

#include <Arduino.h>
#include <WiFiManager.h>
#include <NTPClient.h>
#include <Crypto.h>
#include <ArduinoJson.h>
#include "Asvin.h"

#define DEBUG_MY_UPDATE

#ifndef DEBUG_MY_UPDATE
#define DEBUG_MY_UPDATE(...) Serial.printf( __VA_ARGS__ )
#endif

 // UTC offset
const long utcOffsetInSeconds = 0;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);


// Get credentials from credentials.h
String customer_key = "your-customer_key";
String device_key = "your-device_key";


String firmware_version = "1.0.0";
bool device_registered = false;


/*
This sketch uses wifi manager to manage WiFi credentials
*/

void setup()
{
  Serial.begin(115200); //Serial connection
  delay(500);
  Serial.print("Firmware version: ");
  Serial.println(firmware_version);
  WiFiManager wifiManager;
  // Uncomment below code to reset onboard wifi credentials
  // wifiManager.resetSettings();
  wifiManager.autoConnect("AutoConnectAP");
  timeClient.begin();
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);   // turn the LED on (HIGH is the voltage level)
}

void showDeserializeError(DeserializationError error) {
  Serial.print(F("deserializeJson() failed: "));
  Serial.println(error.c_str());
}

void loop() {
  /*
    -Connect to WiFi
      -Login To Auth Server
        -Register Device
          -CheckRollout
            -Get CID from BlockChain server
              -Download Firmware from IPFS server and update the ESP
                -Check if Rollout was sucessful

  */
  if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status
    DEBUG_MY_UPDATE("Wifi Connected !");
    String mac = WiFi.macAddress();
    //Serial.println(" Getting Time from NTP server ");
    timeClient.update();

    using namespace experimental::crypto;
    unsigned long timestr = timeClient.getEpochTime();
    char payloadBuf[42];
    ltoa(timestr, payloadBuf, 10);

    strcat(payloadBuf, device_key.c_str());

    uint8_t derivedKey[customer_key.length()];
    std::copy(customer_key.begin(), customer_key.end(), derivedKey);

    String device_signature = SHA256::hmac(payloadBuf, derivedKey, sizeof derivedKey, SHA256::NATURAL_LENGTH);
    device_signature.toLowerCase();
    // Serial.println(device_signature);

    delay(2000);
    HTTPClient http;
    Asvin asvin;
    int httpCode;

    // ......Get OAuth Token.................. 
    Serial.println("--Get OAuth Token ");
    String response = asvin.authLogin(device_key, device_signature, timestr, httpCode);
    //Serial.println(" Parsing Auth Code ");
    DynamicJsonDocument doc(1000);
    DeserializationError error = deserializeJson(doc, response);
    if (error)
    {
      showDeserializeError(error);
      return;
    }
    if (httpCode == 200) {
      Serial.println("OAuth Token: OK");
      String authToken = doc["token"];
      //Serial.println(authToken);


      // Register Device 
      if (device_registered) {
        httpCode = 200;
      }
      else {
        //Serial.println("--Register Device");
        String device_name = "esp-conti-demo";
        String result = asvin.RegisterDevice(device_name, mac, firmware_version, authToken, httpCode);
        char buff[result.length() + 1];
        result.toCharArray(buff, result.length() + 1);
        /*
        Serial.print("Buffer --> ");
        Serial.println(buff);
        Serial.println(httpCode);*/
      }
      if (httpCode == 200) {
        device_registered = true;
        //Serial.println("Device Registered: OK ");

        //  CheckRollout
        String resultCheckout = asvin.CheckRollout(mac, firmware_version, authToken, httpCode);
        Serial.println("--Check Next Rollout");
        char buff[resultCheckout.length() + 1];
        resultCheckout.toCharArray(buff, resultCheckout.length() + 1);
        //Serial.println(buff);

        if (httpCode == 200) {
          Serial.println("Next Rollout: OK");
          DynamicJsonDocument doc(1000);
          DeserializationError error = deserializeJson(doc, resultCheckout);

          if (error)
          {
            showDeserializeError(error);
            return;
          }
          String firmwareID = doc["firmware_id"];
          String rolloutID = doc["rollout_id"];
          if (rolloutID == "null") {
            Serial.println("No Rollout available");
            delay(1000 * 3);
            return;
          }

          // Get CID from BlockChain server
          Serial.println("--Get Firmware Info from Blockchain");
          String CidResponse = asvin.GetBlockchainCID(firmwareID, authToken, httpCode);
          char cidbuff[CidResponse.length() + 1];
          CidResponse.toCharArray(cidbuff, CidResponse.length() + 1);
          //Serial.println(cidbuff);

          if (httpCode == 200) {
            Serial.println("Get Firmware Info : OK");
            DynamicJsonDocument doc(500);
            DeserializationError error = deserializeJson(doc, CidResponse);
            if (error)
            {
              showDeserializeError(error);
              return;
            }
            String cid = doc["cid"];
            if (cid.length() == 0) {
              Serial.println("No CID available");
              return;
            }

            // -Download Firmware from IPFS server
            Serial.println("--Download Firmware from IPFS and Install");
            t_httpUpdate_return ret = asvin.DownloadFirmware(authToken, cid);
            switch (ret)
            {
            case HTTP_UPDATE_FAILED:
              Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
              break;
            case HTTP_UPDATE_NO_UPDATES:
              Serial.println("HTTP_UPDATE_NO_UPDATES");
              break;
            case HTTP_UPDATE_OK:
              Serial.println("Update successfull");

              // check if rollout successfull 
              //Serial.println("--Update Rollout");
              String resultCheckout = asvin.CheckRolloutSuccess(mac, firmware_version, authToken, rolloutID, httpCode);
              char buff[resultCheckout.length() + 1];
              resultCheckout.toCharArray(buff, resultCheckout.length() + 1);
              //Serial.println(buff);
              if (httpCode == 200) {
                //Serial.println("Update Rollout : OK");
                Serial.println("--Restart Device and Apply Update : OK");
                ESP.restart();
              }
              else {
                Serial.println("Rollout Update Error!!");
              }
              break;
            }
          }
          else {
            DEBUG_MY_UPDATE("IPFS Error!!");
          }

        }
        else {
          DEBUG_MY_UPDATE("Next Rollout Error!!");
        }
      }
      else {
        DEBUG_MY_UPDATE("Device Registration Error!!");
      }
    }
    else {
      DEBUG_MY_UPDATE("OAuth Error!!");
    }
  }
  else {
    DEBUG_MY_UPDATE("Problem with WiFi!!");
  }
  delay(1000 * 3);
}
