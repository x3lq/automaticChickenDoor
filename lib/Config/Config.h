//
// Created by Flo on 21.09.20.
//

#pragma once
#include <string>
using namespace std;

class Config
{
public:
    void saveConfig();
    void readConfig();

    string ssid() { return ssid_; }
    string pw() { return pw_; }
    string version() { return version_; }
    int state() { return state_; }
    string ip() { return ip_; }
    int openValue() {return openValue_; }
    int closeValue() { return closeValue_; }
    float nTurns() { return nTurns_; }


    void ssid(string id) { ssid_ = id; }
    void pw(string password) { pw_ = password; }
    void versionCode(string version) { version_ = version; }
    void state(int newState) { state_ = newState; } // 0 = close 1 = open
    void ip(string newIp) { ip_ = newIp;}
    void openValue(int newVal) { openValue_ = newVal; }
    void closeValue(int newVal) { closeValue_ = newVal; }
    void turns(float newVal) { nTurns_ = newVal; }



private:
    const char *LOG_TAG = "CONFIG_FILE";
    string ssid_;
    string pw_;
    string version_;
    int state_;
    string ip_;
    int openValue_;
    int closeValue_;
    float nTurns_;

    void setupConfig();
};