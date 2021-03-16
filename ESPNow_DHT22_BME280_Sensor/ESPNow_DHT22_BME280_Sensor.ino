//
// Sample sketch which uses either a BME280 or DHT22 sensor and sends the sensed
// data via ESPNow to an ESPNow gateway. Deep sleep is used to minimze
// current usage. For both sensors an additional GPIO can be used to turn off and on
// Vcc to save more power. Delays can be specified after Vcc activation and after sensor startup
//
// Don't forget to connect GPIO16 with RST. Otherwise there will be no deep sleep wakeup
//
// Latest code available on https://github.com/framps/ESP_stuff/tree/main/ESPNow_DHT22_BME280_Sensor
// Corresponding ESPNow gateway code is available on https://github.com/framps/ESP_stuff/tree/main/ESPNow_gateway
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

/*

Sample log

19:50:25.260 -> Enable DHT
19:50:26.253 -> Initialized DHT22
19:50:26.253 -> Mac address of sensor: 10:52:1C:02:44:C7
19:50:26.253 -> ... loop ...
19:50:26.286 -> Temp: 40.000000, Hum: 18.000000
19:50:26.286 -> send, hum=40.00
19:50:26.286 -> send, temp=18.00
19:50:26.286 -> send_cb, status = 0, to mac: 10:52:1C:5D:5C:9D
19:50:26.286 -> Going to sleep, uptime: 1087
*/

#include "TempHumSensor.h"
#include "ESPNow.h"

#define GATEWAY_MAC { 0x10, 0x52, 0x1C, 0x5D, 0x5C, 0x9D}
#define DHT_PIN 12                 // GPIO used to retrieve DHT sensor data
#define POWER_PIN 15               // Vcc power pin, used to turn off Vcc of sensors before deep sleep starts and
                                   // to turn on when woken up from deep sleep
#define POWERDOWN_PIN 14           // GPIO pin connected to chip enable (CH_PD/Enable) to power down ESP when Vcc too low 
#define POWERDOWN_VCC 3000         // vcc in mV when to shutdown ESP

ADC_MODE(ADC_VCC);

// #define BME280_SENSOR // otherwise use DHT22 sensor

#define DEBUG                      // enable debug messages
                                
// mac of ESPNow gateway
uint8_t gatewayMac[] = GATEWAY_MAC;

Sensor *s = NULL;

// experimental ! Tests not completed as of now
ESPNow::PowerDownConfig powerDownConfig{POWERDOWN_PIN, POWERDOWN_VCC}; 

// ESPNow has following default values:
// wifiChannel:  1, NOTE: wifiChannel has to be constant all the time and doesn't float
// deep sleep time: 60 seconds
// ESPNow send timeout: 10 seconds
ESPNow* e= new ESPNow(gatewayMac);          // create ESPNow singleton

// experimental ! Tests not completed as of now
// ESPNow* e= new ESPNow(gatewayMac,1,60e6,10000,&powerDownConfig);    // create ESPNow singleton

ESPNow* ESPNow::instance = e;                // make singleton global accessible for ESP callback

void setup() {

    Serial.begin(115200);
    Serial.println();

#ifdef BME280_SENSOR
    s = new BME280Sensor();                      // no powerpin usage
    // s = new BME280Sensor(POWER_PIN);          // use default delays 
    // s = new BME280Sensor(POWER_PIN, Sensor::Delays{100,100});   // use custom delays
#else
    s = new DHT22Sensor(DHT_PIN);                // no powerpin usage
    // s = new DHT22Sensor(DHT_PIN, POWER_PIN);  // use default delays
    // s = new DHT22Sensor(DHT_PIN, POWER_PIN, Sensor::Delays{500,10});   // use custom delays
#endif

#ifdef DEBUG  
    Serial.println("Enabling debug messages");
    s->enableDebug();
    e->enableDebug();
#endif    

//  start sensor

    if (! s->start()) {
      Serial.printf("Initialization failed for %s\n",s->name());
      delay(3000);
      ESP.restart();
  } else {
      Serial.printf("Initialized %s\n",s->name());
  }

//  start ESPNow

   e->start();
}

void loop() {

//  Poll sensor data

    if (! s->poll()) {
        Serial.println("Poll failed");
        delay(3000);
        ESP.restart();
    };

    Serial.printf("Temperature: %f, Humidity: %f\n",s->temperature(), s->humidity());

//  send sensor data via ESPNow    

    if (! e->send(*s)) {
        Serial.println("Send failed");
        delay(3000);
        ESP.restart();
    }

//  stop sensor

    s->stop();              

//  and shutdown ESP into deep sleep

    e->shutdown();
}
