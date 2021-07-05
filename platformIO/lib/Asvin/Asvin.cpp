/**
 * asvin.cpp
 * @author Rohit Bohara
 * 
 * Copyright (c) 2019 asvin.io. All rights reserved. 
 */
#include "Asvin.h"
#include <Arduino.h>


 
Asvin::Asvin(void){

}
Asvin::~Asvin(void){
    
}


/**
 
 Gets the server fingerprints from the Asvin Tools server

@param httpCode.
@return the numerical value of the color code.
  */

String Asvin::getFingerprints(int& httpCode){
    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    client->setInsecure();
    HTTPClient http;
    http.begin(*client, asvinToolsURL);
    http.addHeader(F("Content-Type"), "application/json");
    String payload;
    char buff[payload.length()+1];
    payload.toCharArray(buff, payload.length()+1);
    DEBUG_ASVIN_UPDATE("[asvin tools server] Get Fingerprints : %s\n", buff);
    httpCode = http.POST(payload);   //Send the request
    delay(1000);
    String res = http.getString();  //Get the response payload
    // Serial.println(res);
    delay(1000);
    http.end();  //Close connection to asvin server

    DynamicJsonDocument doc(500);
    DeserializationError error = deserializeJson(doc, res);
    
    fingerprintBC = doc["bc"].as<String>();
    fingerprintIPFS = doc["ipfs"].as<String>();
    fingerprintVC = doc["vc"].as<String>();
    fingerprintAuth = doc["oauth"].as<String>();

  
    if (error)
    {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return "";
    }
  
    
    Serial.println("Added fingerpints");

    return res;
}

/**
 
 Gets the auth token from the Asvin Oauth server

@param  String deviceKey - from credentials.h file 
        String deviceSignature - derived from customer key + device key  
        long unsigned int timestamp - get time stamp from NTP server 
        int& httpCode.
@return the auth token.
  */


String Asvin::authLogin(String deviceKey, String deviceSignature, long unsigned int timestamp, int& httpCode){
    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    Serial.print("Fingerprint auth: ");
    Serial.println(fingerprintAuth);
    client->setFingerprint(fingerprintAuth.c_str());
    HTTPClient http;
    http.begin(*client, authserverLoginURL);
    http.addHeader(F("Content-Type"), "application/json");
    DynamicJsonDocument doc(500);
    doc["device_key"] = deviceKey;
    doc["device_signature"] = deviceSignature;
    doc["timestamp"] = timestamp;
    String payload;
    serializeJson(doc, payload);
    char buff[payload.length()+1];
    payload.toCharArray(buff, payload.length()+1);
    DEBUG_ASVIN_UPDATE("[asvin auth server] Auth Server Login : %s\n", buff);
    httpCode = http.POST(payload);   //Send the request
    delay(1000);
    String res = http.getString();  //Get the response payload
    delay(1000);
    http.end();  //Close connection to asvin server
    return res;
}

/**
 
 Register Device on Asvin platform

@param  const String mac - Mac address of this device
        String currentFwVersion - current firmware version on this code
        String token - get from authLogin()
        int& httpCode.
@return device registration status.
  */

String Asvin::registerDevice(const String mac, String currentFwVersion, String token, int& httpCode){
    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    Serial.print("Fingerprint VC: ");
    Serial.println(fingerprintVC);
    client->setFingerprint(fingerprintVC.c_str());
    HTTPClient http;
    http.begin(*client, registerURL);
    http.addHeader(F("Content-Type"), "application/json");
    http.addHeader(F("x-access-token"), token);
    DynamicJsonDocument doc(500);
    doc["mac"] = mac;
    doc["firmware_version"] = currentFwVersion;
    String payload;
    serializeJson(doc, payload);
    char buff[payload.length()+1];
    payload.toCharArray(buff, payload.length()+1);
    DEBUG_ASVIN_UPDATE("[asvin Version Controller] Register Device  : %s\n", buff);
    httpCode = http.POST(payload);   //Send the request
    Serial.println("Posted ...");
    delay(1000);
    String res = http.getString();  //Get the response payload
    http.end();  //Close connection to asvin server
    return res;
}

/**
 
 Check for Firmware rollout from the Asvin server

@param  const String mac - Mac address of this device
        String currentFwVersion - current firmware version on this code
        String token - get from authLogin()
        int& httpCode.
@return rollout status
  */
String Asvin::checkRollout(const String mac, const String currentFwVersion, String token, int& httpCode){
    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    Serial.print("Fingerprint VC: ");
    Serial.println(fingerprintVC);
    client->setFingerprint(fingerprintVC.c_str());
    HTTPClient http;
    http.begin(*client, checkRolloutURL);
    http.addHeader(F("Content-Type"), "application/json");
    http.addHeader(F("x-access-token"), token);
    DynamicJsonDocument doc(500);
    doc["mac"] = mac;
    doc["firmware_version"] = currentFwVersion;
    String payload;
    serializeJson(doc, payload);
    char buff[payload.length()+1];
    payload.toCharArray(buff, payload.length()+1);
    DEBUG_ASVIN_UPDATE("[asvinUpdate] Next RollOut ---> : %s\n", buff);
    httpCode = http.POST(payload); //Send the request
    delay(1000);  
    String res = http.getString();  //Get the response payload
    delay(1000);
    http.end();  //Close connection to asvin server
    return res;
}

/**
 
 Check for Content ID(CID) from Block Chain server

@param  const String firmwareID - current firmware ID
        String token - get from authLogin()
        int& httpCode.
@return blockchain CID
  */
String Asvin::getBlockchainCID(const String firmwareID, String token, int& httpCode){
    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    Serial.print("Fingerprint BC: ");
    Serial.println(fingerprintBC);
    client->setFingerprint(fingerprintBC.c_str());
    HTTPClient http;
    http.begin(*client, bcGetFirmwareURL);
    http.addHeader(F("Content-Type"), "application/json");
    http.addHeader(F("x-access-token"), token);
    DynamicJsonDocument doc(256);
    doc["id"] = firmwareID;
    String payload;
    serializeJson(doc, payload);
    char buff[payload.length()+1];
    payload.toCharArray(buff, payload.length()+1);
    DEBUG_ASVIN_UPDATE("[asvinUpdate] Blockchain Login : %s\n", buff);
    httpCode = http.POST(payload);   //Send the request
    delay(1000);
    String res = http.getString();  //Get the response payload
    delay(1000);
    http.end();  //Close connection to asvin server
    return res;
}

/**
 
 Check for sucess of a Firmware rollout 

@param  const String mac - Mac address of this device
        String currentFwVersion - current firmware version on this code
        String token - get from authLogin()
        const String rolloutID - Id of the firmware rollout 
        int& httpCode.
@return rollout success status
  */

String Asvin::checkRolloutSuccess(const String mac, const String currentFwVersion, String token, const String rolloutID, int& httpCode){
    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    Serial.print("Fingerprint VC: ");
    Serial.println(fingerprintVC);
    client->setFingerprint(fingerprintVC.c_str());
    HTTPClient http;
    http.begin(*client, checkRolloutSuccessURL);
    http.addHeader(F("Content-Type"), "application/json");
    http.addHeader(F("x-access-token"), token);
    DynamicJsonDocument doc(256);
    doc["mac"] = mac;
    doc["firmware_version"] = currentFwVersion;
    String payload;
    serializeJson(doc, payload);
    char buff[payload.length()+1];
    payload.toCharArray(buff, payload.length()+1);
    DEBUG_ASVIN_UPDATE("[asvinUpdate] Check Rollout Success  : %s\n", buff);
    httpCode = http.POST(payload);   //Send the request
    delay(1000);
    String res = http.getString();  //Get the response payload
    delay(1000);
    http.end();  //Close connection to asvin server
    return res;
}

/**
 
 Download the firmware form IPFS server and update firmware on the device 

@param  String token - get from authLogin()
        const String cid - CID from the block chain server
@return t_httpUpdate_return
  */
t_httpUpdate_return Asvin::downloadFirmware(String token, const String cid){
    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);
    Serial.print("Fingerprint IPFS: ");
    Serial.println(fingerprintIPFS);
    client->setFingerprint(fingerprintIPFS.c_str());
    StaticJsonDocument<80> doc;
    doc["cid"] = cid;
    String payload;
    const String currentVersion = "1.0.0";
    serializeJson(doc, payload);
    char buff[payload.length()+1];
    payload.toCharArray(buff, payload.length()+1);
    DEBUG_ASVIN_UPDATE("[asvinUpdate] Download firmware HTTP payload : %s\n", buff);
    t_httpUpdate_return res = ESPhttpUpdate.update(*client, ipfsDownloadURL, payload, token, currentVersion);
    return res;
}

