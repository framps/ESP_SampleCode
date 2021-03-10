//
// Sample sketch which uses either a BME280 or DHT22 sensor and sends the sensed
// data via ESPNow to an ESPNow gateway. Deep sleep is used to minimze
// current usage. For both sensors an additional GPIO can be used to turn off and on
// power pin to save more current. 
//
// Don't forget to connect GPIO16 with RST. Otherwise there will be no deep sleep wakeup
//
// Latest code available on https://github.com/framps/ESP_stuff/tree/main/DHT22-BME280-espnow-deepSleep-sensor
//

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

#include "TempHumSensor.h"
#include "ESPNow.h"

#define DHTPIN 12               // D2 is GPIO 4
#define DHTPower 15             // Vcc power pin D8
#define DHTTYPE DHT22           // DHT 22  (AM2302), AM2321
#define BMEPower DHTPower       // Vcc power pin D8 

// #define BME280 // otherwise DHT22 is assumed

uint8_t gatewayMac[] = { 0x10, 0x52, 0x1C, 0x5D, 0x5C, 0x9D};

Sensor *s = NULL;

ESPNow* e= new ESPNow(gatewayMac);          // create ESPNow singleton
ESPNow* ESPNow::instance = e;               // make singleton globally accessible

void setup() {

    Serial.begin(115200);
    Serial.println("... setup ...");

#ifdef BME280  
    s = new BME280Sensor(BMEPower);
#else
    s = new DHT22Sensor(DHTPIN, DHTTYPE, DHTPower);
#endif
    
    if (! s->start()) {
      Serial.printf("Initialization failed for %s\n",s->name());
      delay(3000);
      ESP.restart();
  } else {
      Serial.printf("Initialized %s\n",s->name());
  }

   e->initialize();
}

void loop() {

    Serial.println("... loop ...");

    Sensor::Data sensorData;

    if (! s->poll(sensorData)) {
        Serial.println("Poll failed");
        delay(3000);
        ESP.restart();
    };

    Serial.printf("Temp: %f, Hum: %f\n",sensorData.temp, sensorData.hum);

    if (! e->send(sensorData)) {
        Serial.println("Send failed");
        delay(3000);
        ESP.restart();
    }

    e->shutdown();
}
