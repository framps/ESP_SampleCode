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

// Latest code available on https://github.com/framps/ESP_stuff/tree/main/DHT22-BME280-espnow-deepSleep-sensor

#include "ESPNow.h"
#include <ESP8266WiFi.h>
extern "C" {
#include <espnow.h>
}

ESPNow::ESPNow(uint8_t* gatewayMac, int wifiChannel, int sleepTime, int sendTimeout) : gatewayMac(gatewayMac), wifiChannel(wifiChannel), sleepTime(sleepTime), sendTimeout(sendTimeout), dataSent(false) { };

int ESPNow::initialize() { 

  ESPNow::instance = this;
  WiFi.mode(WIFI_STA);                // Station mode for sensor
  WiFi.begin();
  Serial.print("Mac address of sensor: "); Serial.println(WiFi.macAddress());

  if (esp_now_init() != 0) {
    Serial.println("ESPNow init failed");
    delay(3000);
    ESP.restart();
  }

  delay(10); 

  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_add_peer(this->gatewayMac, ESP_NOW_ROLE_SLAVE, this->wifiChannel, NULL, 0);

  esp_now_register_send_cb([](uint8_t* mac, uint8_t status) {
    Serial.print("send_cb, status = "); Serial.print(status);
    Serial.print(", to mac: ");
    char macString[50] = {0};
    sprintf(macString, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    Serial.println(macString);
    ESPNow::instance->dataSent=true;
  });
}

int ESPNow::send(Sensor::Data &polledData) { 

    Serial.print("send, hum="); Serial.println(polledData.hum);
    Serial.print("send, temp="); Serial.println(polledData.temp);
  
    u8 bs[sizeof(polledData)];
    memcpy(bs, &polledData, sizeof(polledData));
    esp_now_send(NULL, bs, sizeof(bs)); 

    int rc = this->waitForCompletion();
    this->dataSent = false;
    return rc;
  }

int ESPNow::waitForCompletion() {
  while ( ! this->dataSent && millis() <= this->sendTimeout ) {
    delay(1);
  }
  return this->dataSent;
}

void ESPNow::shutdown() { 
    Serial.print("Going to sleep, uptime: "); Serial.println(millis());
    ESP.deepSleep(this->sleepTime, WAKE_RF_DEFAULT);
    delay(100);       // give ESP time to complete shutdown
    // no return
}
