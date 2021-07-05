/*
 * Esp8266HttpOta
 *
 * The sketch was developed for a prototype consist of Nodemcu ESP8266 board
 * and 1602A LCD board. The board connects to asvin platform and perform OTA 
 * firmware updates. 
 * 
 * Written by Rohit Bohara, Apache-2.0 License
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Stream.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include "Asvin.h"
// WifiManager Dependancies
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <NTPClient.h>
#include <Crypto.h>
#include <TypeConversion.h>

// UTC offset
const long utcOffsetInSeconds = 0;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);


// Get credentials from credentials.h
String customer_key = "";
String device_key = "";

String key = "3";
String firmware_version = "1.0.0";


/*
This sketch uses wifi manager to manage WiFi credentials
*/

void setup()
{
  Serial.begin(115200); //Serial connection
  delay(500);
  WiFiManager wifiManager;
  // Uncomment below code to reset onboard wifi credentials
  // wifiManager.resetSettings();
  wifiManager.autoConnect("AutoConnectAP");
  timeClient.begin();

}

void loop(){
  /*
    -Connect to WiFi
      -Login To Auth Server
        -Register Device 
          -CheckRollout
            -Get CID from BlockChain server
              -Download Firmware from IPFS server and update the ESP
                -Check if Rollout was sucessful 
    
  */

  if (WiFi.status() == WL_CONNECTED){ //Check WiFi connection status
      Serial.println(" Wifi Connected !");

      Serial.println(" Getting Time from NTP server ");
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
      String mac = WiFi.macAddress();
      Asvin asvin;
      int httpCode;
            
      // ......Get OAuth Token.................. 
      Serial.println("Get OAuth Token ");
      
      String response = asvin.authLogin(device_key, device_signature, timestr, httpCode);
      Serial.println(" Parsing Auth Code ");
      DynamicJsonDocument doc(1000);
      DeserializationError error = deserializeJson(doc, response);
      String authToken = doc["token"];
      if (error)
      {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
        return;
      }

      Serial.println(authToken);

      // Register Device 

      String result = asvin.RegisterDevice(mac, firmware_version, authToken, httpCode);
      char buff[result.length() + 1];
      result.toCharArray(buff, result.length() + 1);
      Serial.print("Buffer --> ");
      Serial.println(buff);
      Serial.println(httpCode);
      
      if (httpCode == 200){
        Serial.println(" Device Registered: OK ");

        //  -CheckRollout
        String resultCheckout = asvin.CheckRollout(mac, firmware_version, authToken, httpCode);
        Serial.println("checkRollout");
        char buff[resultCheckout.length() + 1];
        resultCheckout.toCharArray(buff, resultCheckout.length() + 1);
        Serial.println(buff);
        
        if (httpCode == 200){
          Serial.println("http 200: Next Rollout: OK");
          DynamicJsonDocument doc(1000);
          DeserializationError error = deserializeJson(doc, resultCheckout);

          if (error)
          {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.c_str());
            return;
          }
            String firmwareID = doc["firmware_id"];
            String rolloutID = doc["rollout_id"];
            // -Get CID from BlockChain server
            String CidResponse = asvin.GetBlockchainCID(firmwareID, authToken, httpCode);
            char cidbuff[CidResponse.length() + 1];
            CidResponse.toCharArray(cidbuff, CidResponse.length() + 1);
            Serial.println(cidbuff);

            if (httpCode == 200) {
              Serial.println("http 200: CID Resonse : OK");
              DynamicJsonDocument doc(500);
              DeserializationError error = deserializeJson(doc, CidResponse);
              if (error)
                {
                  Serial.print(F("deserializeJson() failed: "));
                  Serial.println(error.c_str());
                  return;
                }
              String cid = doc["cid"];
              // -Download Firmware from IPFS server
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
                Serial.println("HTTP_UPDATE_OK");
                break;
              }
              if (httpCode == 200){
                Serial.println("Firmware updated");
              }
              // if (httpCode == 200)
              // {
              //   // check if rollout successfull 
              //   Serial.println("http 200");
              //   String resultCheckout = asvin.CheckRolloutSuccess(mac, firmware_version, authToken, rolloutID, httpCode);
              //   char buff[resultCheckout.length() + 1];
              //   resultCheckout.toCharArray(buff, resultCheckout.length() + 1);
              //   Serial.println(buff);
              // }
            }

          // -Check if Rollout was sucessful 
        }
      }
    }
  }
