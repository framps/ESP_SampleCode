/*
#######################################################################################################################
#
#    Copyright (c) 2021 framp at linux-tips-and-tricks dot de
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#######################################################################################################################
*/

// open weather map code used as template from Emmanuel Odunlade from
// https://randomnerdtutorials.com/esp8266-nodemcu-http-get-open-weather-map-thingspeak-arduino/ 
// and modfied to retrieve the required data. In addition the code was migrated from
// ArduinoJson V5 to ArduinoJson V6.

 /*
  * Author: Emmanuel Odunlade 
  * Complete Project Details http://randomnerdtutorials.com
  */

#include "homedefs.h"         // WLAN and OWM definitions 
  
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>

// Replace with your SSID and password details
char ssid[] = AP_NAME;        
char pass[] = AP_PASSWORD;

const float pressureNN=1035.25;
float pressureOWM=0;

WiFiClient client;

// Open Weather Map API server name
const char owmServer[] = "api.openweathermap.org";

// Replace the next line to match your city and 2 letter country code
String nameOfCity = OWM_CITY; 

// Replace the next line with your API Key
String apiKey = OWM_API_KEY;

// Replace the next line with your longitude and lattitude
String lon = OWM_LON;
String lat= OWM_LAT;


String text;

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>

Adafruit_BMP280 bmp; // use I2C interface
Adafruit_Sensor *bmp_temp = bmp.getTemperatureSensor();
Adafruit_Sensor *bmp_pressure = bmp.getPressureSensor();

int jsonend = 0;
boolean startJson = false;
int status = WL_IDLE_STATUS;

#define JSON_BUFF_DIMENSION 2500

unsigned long lastConnectionTime = 10 * 60 * 1000;  // last time you connected to the server, in milliseconds
const unsigned long postInterval = 10 * 60 * 1000;  // posting interval of 10 minutes  (10L * 1000L; 10 seconds delay for testing)

void setup() {

  Serial.begin(115200);

  if (!bmp.begin()) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
    while (1) delay(10);
  }

 /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_FORCED,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

  bmp_temp->printSensorDetails();
  
  text.reserve(JSON_BUFF_DIMENSION);
  
  WiFi.begin(ssid,pass);
  Serial.println("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi Connected");
  printWiFiStatus();
}

void loop() { 
  //OWM requires 10mins between request intervals
  //check if 10mins has passed then conect again and pull
  if (millis() - lastConnectionTime > postInterval) {
    // note the time that the connection was made:
    lastConnectionTime = millis();
    makehttpRequest();
  }
  
  sensors_event_t temp_event, pressure_event;
  bmp_temp->getEvent(&temp_event);
  bmp_pressure->getEvent(&pressure_event);
  
  Serial.print(F("Temperature = "));
  Serial.print(temp_event.temperature);
  Serial.println(" °C");

  Serial.print(F("Pressure = "));
  Serial.print(pressure_event.pressure);
  Serial.println(" hPa");

  Serial.print(F("Approx altitude = "));
  Serial.print(bmp.readAltitude(pressureNN)); 
  Serial.println(" m");

  delay(postInterval);
  
}

// print Wifi status
void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

// to request data from OWM
void makehttpRequest() {
  // close any connection before send a new request to allow client make connection to server
  client.stop();

  // if there's a successful connection:
  if (client.connect(owmServer, 80)) {
    // send the HTTP PUT request:
    char request[512];
    sprintf(request,"GET /data/2.5/weather?lat=%s&lon=%s&APPID=%s&mode=json&units=metric HTTP/1.1",lat.c_str(),lon.c_str(),apiKey.c_str());
    Serial.println("connecting...");
    // Serial.println(request);
    
    client.println(String(request));
    client.printf("Host: %s\n",owmServer);
    client.println("User-Agent: MyArduinoWifiClient/1.1");
    client.println("Connection: close");
    client.println();
    
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        Serial.println(">>> Client Timeout !");
        client.stop();
        return;
      }
    }

    delay(1000);  // missing in original code. Otherwise response will be truncated.
  
    char c = 0;
    while (client.available() ) {
      c = client.read();
      // since json contains equal number of open and close curly brackets, this means we can determine when a json is completely received  by counting
      // the open and close occurences,
      // Serial.print(c);
      if (c == '{') {
        startJson = true;         // set startJson true to indicate json message has started
        jsonend++;
      }
      if (c == '}') {
        jsonend--;
      }
      if (startJson == true) {
        text += c;
      }
      // if jsonend = 0 then we have have received equal number of curly braces 
      if (jsonend == 0 && startJson == true) {
        parseJson(text.c_str());  // parse c string text in parseJson function
        text = "";                // clear text string for the next time
        startJson = false;        // set startJson to false to indicate that a new message has not yet started
      }
    }
  }
  else {
    // if no connction was made:
    Serial.println("connection failed");
    return;
  }
}

//to parse json data received from OWM
void parseJson(const char * jsonString) {

  // Serial.printf("JSON: %s\n",jsonString);
  
  const size_t bufferSize = 4096;
  DynamicJsonDocument jsonDoc(bufferSize);

  // FIND FIELDS IN JSON TREE
  auto error = deserializeJson(jsonDoc, jsonString);
  if (error) {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(error.c_str());
    return;
  }

  JsonObject root = jsonDoc.as<JsonObject>();
  
  // including temperature and humidity for those who may wish to hack it in
  
  float tempNow = root["main"]["temp"];
  float humNow = root["main"]["humidity"];
  float pressNow = root["main"]["pressure"];

  Serial.print("OWM Temp: ");
  Serial.print(tempNow);
  Serial.println(" °C");
  Serial.print("OWM Hum: ");
  Serial.print(humNow);
  Serial.println(" %");
  Serial.print("OWM Press: ");
  Serial.print(pressNow);
  Serial.println(" hPa");

  pressureOWM=pressNow;   
}
