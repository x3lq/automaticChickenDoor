#include <Arduino.h>
#include "SPIFFS.h"
#include "Config.h"
#include <string>
#include "Motor.h"
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <FS.h>
#include <Stepper.h>
#include "DHT.h"

#define TEMP_PIN_INSIDE 33
#define TEMP_PIN_OUTSIDE 34
#define DHTTYPE DHT11

Motor motor;
AsyncWebServer server(80);
Config config;
DHT tmpInside(TEMP_PIN_INSIDE, DHTTYPE);
DHT tmpOutside(TEMP_PIN_OUTSIDE, DHTTYPE);

int sensorValue = 10; //current photoresistor reading
int closeValue = 0.2;  //closing light
int openValue = 0.7; //opening light

int openDoor = 0;
int closeDoor = 0;

int blocked = 0;
long blockTimer = 10000;
long blockStart = 0;

int gateState = 0; // 0= closed, 1=open
float neededTurns = 3;

float tempInsideReading = 0.0;
float tempOutsideReading = 0.0;

Stepper stepper(64, 12, 13, 27, 14);

void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Not found");
}

void printFile(const char *filename)
{
  // Open file for reading
  File file = SPIFFS.open(filename);
  if (!file)
  {
    Serial.println(F("Failed to read file"));
    return;
  }

  // Extract each characters by one by one
  while (file.available())
  {
    Serial.print((char)file.read());
  }
  Serial.println();

  // Close the file
  file.close();
}

void setupWebserver()
{
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, "/index.html");
    response->addHeader("IP-Adress", config.ip().c_str());
    request->send(response);
  });

  // Send a GET request to <IP>/get?message=<message>
  server.on("/getSensorReading", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "" + String((float) sensorValue / 4095.0));
  });

  server.on("/getMotorTurns", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "" + String(neededTurns));
  });

  server.on("/getState", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "" + String(gateState));
  });

  server.on("/getTempInside", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "" + String(tempInsideReading));
  });

  server.on("/getTempOutside", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "" + String(tempOutsideReading));
  });

  // Send a POST request to <IP>/post with a form field message set to <message>
  server.on("/setMotorTurns", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("steps", true))
    {
      neededTurns = request->getParam("steps", true)->value().toFloat();
      config.turns(neededTurns);
      config.saveConfig();
      return request->send(200, "text/plain", "Set neededTurns to " + String(neededTurns));
    }

    request->send(400, "text/plain", "Missing steps");
  });

  server.on("/open", HTTP_POST, [](AsyncWebServerRequest *request) {
    if(blocked == 1) {
      return request->send(400, "text/plain", "Motor already working)");
    }
    openDoor = 1;
    request->send(200, "text/plain", "Opening");
  });

  server.on("/close", HTTP_POST, [](AsyncWebServerRequest *request) {
    if(blocked == 1) {
      return request->send(400, "text/plain", "Motor already working)");
    }
    closeDoor = 1;
    request->send(200, "text/plain", "Closing");
  });

  server.on("/setDayReading", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("value", true))
    {
      openValue = request->getParam("value", true)->value().toInt();
      config.openValue(openValue);
      config.saveConfig();
      return request->send(200, "text/plain", "OpenValue set to " + openValue);
    }

    request->send(400, "text/plain", "Missing openValue");
  });

  server.on("/setNightReading", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("value", true))
    {
      closeValue = request->getParam("value", true)->value().toInt();
      config.closeValue(closeValue);
      config.saveConfig();
      String message = "Close value set to " + closeValue;
      return request->send(200, "text/plain", message);
    }

    request->send(400, "text/plain", "Missing value");
  });

  server.on("/reboot", HTTP_POST, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Rebooting");
    ESP.restart();
  });

  server.on("/reset", HTTP_POST, [](AsyncWebServerRequest *request) {
    SPIFFS.remove("config.json");
    config.readConfig();
    request->send(200, "text/plain", "Removed file");

  });

  server.on("/setWlan", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("ssid", true) && request->hasParam("pw", true))
    {
      String ssid = request->getParam("ssid", true)->value();
      String pw = request->getParam("pw", true)->value();

      config.ssid(ssid.c_str());
      config.pw(pw.c_str());
      config.saveConfig();
    
      return request->send(200, "text/plain", "Succesfully set wlan, trying to connect on reboot");
    }

  return request->send(200, "text/plain", "Failed to set wlan settings");
});

}

void listDir(char *dir)
{

  File root = SPIFFS.open(dir);

  File file = root.openNextFile();

  while (file)
  {

    Serial.print("FILE: ");
    Serial.println(file.name());

    file = root.openNextFile();
  }
}

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);

  while (!Serial)
    continue;

  if (!SPIFFS.begin(true))
  {
    return;
  }

  //Test Fiel Syste,
  Serial.println("\n----DIR: /folder1");
  listDir("/");
  Serial.println("");

  config.readConfig();
  openValue = config.openValue();
  closeValue = config.closeValue();
  neededTurns = config.nTurns();
  gateState = config.state();
  printFile("/config.json");

  int connected = 0;
  if(config.ssid().length() != 0) {
      for (int i=0; i<3; i++) {
        Serial.println("Trying to connect to the Wlan");
        WiFi.mode(WIFI_STA);
        WiFi.begin(config.ssid().c_str(), config.pw().c_str());
        if (WiFi.waitForConnectResult() == WL_CONNECTED)
        {
          Serial.printf("WiFi connected!\n");
          Serial.print("IP Address: ");
          Serial.println(WiFi.localIP());
          connected = 1;
          break;
        }
      }
  }

  if(connected == 0) {
    WiFi.softAP("Stall Default AP", "DieNamenMeinerHuehnerSind1234");
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);
  }

  config.ip(WiFi.localIP().toString().c_str());
  config.saveConfig();

  motor.setup();
  tmpInside.begin();
  tmpOutside.begin();

  setupWebserver();
  //DEBUG OUTPUT remove

  server.onNotFound(notFound);
  server.begin();
}

void loop()
{
  //motor.nTurnsCW(15);
  if (blocked == 0)
  {
    if (openDoor == 1)
    {
      Serial.println("Opening");
      motor.nTurnsCCW(neededTurns);
      openDoor = 0;
      gateState = 1;
      blockStart = millis();
      blocked = 1;
      config.state(1);
      config.saveConfig();
    }

    if (closeDoor == 1)
    {
      Serial.println("Close");
      motor.nTurnsCW(neededTurns);
      closeDoor = 0;
      gateState = 0;
      blockStart = millis();
      blocked = 1;
      config.state(0);
      config.saveConfig();
    }
  } else {
      closeDoor = 0;
      openDoor = 0;

      if(millis() < blockTimer + blockStart) {
        //Serial.println("blocked");
        blocked = 1;
      } else {
        //Serial.println("open");
        blocked = 0;
      }
  }

  //Serial.println(sensorValue);
  //Serial.print(" \n"); // prints a space between the numbers
  delay(500); // wait 100ms for next reading

  //for(int i=0; i<40; i++) {
  //  int sensorValue = analogRead(i); // read analog input pin 0
  //  if(sensorValue != 0 && sensorValue != 4095) {
  //    Serial.println(String(i) + " "+ sensorValue);
  //  }
  //}
  sensorValue = analogRead(39);
  float perce = (float)sensorValue / 4095.0;

  /*if (perce < closeValue && config.state() == 1) {
    closeDoor = 1;
  } else if(perce > openValue && config.state() == 0) {
    openDoor = 1;
  } */
  //Serial.println(perce);
  //Serial.println(analogRead(36));

  tempInsideReading = tmpInside.readTemperature();
  tempOutsideReading = tmpOutside.readTemperature();

  Serial.println(tempInsideReading);
  Serial.println(tempInsideReading);
}