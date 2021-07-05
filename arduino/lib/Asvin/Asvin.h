/**
 * asvin.h
 * @author Rohit Bohara
 * 
 * Copyright (c) 2019 asvin.io. All rights reserved. 
 */
#ifndef ASVIN_H_
#define ASVIN_H_

#include <Arduino.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include "ESP8266httpUpdate.h"
#include <WiFiClientSecureBearSSL.h>


#ifndef DEBUG_ASVIN_UPDATE
// #define DEBUG_ASVIN_UPDATE
#define DEBUG_ASVIN_UPDATE(...) Serial.printf( __VA_ARGS__ )
#endif

#ifndef DEBUG_ASVIN_UPDATE
 #define DEBUG_ASVIN_UPDATE(...)
#endif

class Asvin
{
    public:
        Asvin(void);
        ~Asvin(void);
        String RegisterDevice(const String mac, String currentFwVersion, String token, int& httpCode);
        String CheckRollout(const String mac, const String currentFwVersion, String token, int& httpCode);
        String authLogin(String device_key, String device_signature, long unsigned int timestamp, int& httpCode);
        String GetBlockchainCID(const String firmwareID, String token, int& httpCode);
        String CheckRolloutSuccess(const String mac, const String currentFwVersion, String token, const String rollout_id, int& httpCode);
        t_httpUpdate_return DownloadFirmware(String token, const String cid);
        

    private:
        const String registerURL = "";
        const String checkRollout = "";
        const String checkRolloutSuccess = "";
        const String authserver_login = "";
        const String bc_GetFirmware = "";
        const String ipfs_Download = "";
      
};                                 
#endif
