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

#pragma once

#include "TempHumSensor.h"

	class ESPNow {

  const static int WIFI_CHANNEL = 1;
  const static int SLEEP_TIME = 60e6;
  const static int SEND_TIMEOUT = 10000;

  public:

    ESPNow(uint8_t* gatewayMac, int wifiChannel=WIFI_CHANNEL, int sleepTime=SLEEP_TIME, int sendTimeout=SEND_TIMEOUT);
    virtual ~ESPNow() { };
      
    int initialize();                     // rc 0 -> request failed
    int send(Sensor::Data &polledData);   // rc 0 -> request failed
    int waitForCompletion();              // rc 0 -> request failed
    void shutdown();                      // rc 0 -> request failed

    static ESPNow* instance;

  private:
    void sendData(uint8_t* mac, uint8_t status);

    uint8_t* gatewayMac;
    bool dataSent;
    int sleepTime;
    int wifiChannel;
    int sendTimeout;
    };
