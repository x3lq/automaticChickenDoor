//
// Created by Flo on 21.09.20.
//

#include "Config.h"
#include "Arduino.h"
#include "ArduinoJson.h"

#ifdef ESP32

#include <SPIFFS.h>

#endif

void Config::readConfig()
{
    Serial.println("Reading config");

    File file = SPIFFS.open("/config.json");
    StaticJsonDocument<256> doc;

    if(!file) {
        Serial.println("File not found");
        return;
    }

    DeserializationError error = deserializeJson(doc, file);
    if (error)
    {
        Serial.print("File not found, creating new File: ");
        Serial.println(error.c_str());
        setupConfig();
        return;
    }

    
    ssid(doc["ssid"]);
    pw(doc["ssid_pw"]);
    state(doc["state"]);
    versionCode(doc["version"]);
    ip(doc["ip"]);
    openValue(doc["openValue"]);
    closeValue(doc["closeValue"]);
    turns(doc["turns"]);
    file.close();
}

void Config::saveConfig()
{
    File file = SPIFFS.open("/config.json", FILE_WRITE);
    DynamicJsonDocument doc(256);

    doc["ssid"] = ssid();
    doc["ssid_pw"] = pw();
    doc["state"] = state();
    doc["version"] = version();
    doc["ip"] = ip();
    doc["openValue"] = openValue();
    doc["closeValue"] = closeValue();
    doc["turns"] = nTurns();

    serializeJsonPretty(doc, file);
    file.close();
}

void Config::setupConfig()
{
    Serial.println("Setup config");
    ssid("");
    pw("");
    state(1);
    versionCode("1.0");
    ip("");
    
    saveConfig();
}
