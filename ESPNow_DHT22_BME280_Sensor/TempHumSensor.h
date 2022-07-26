//
// A small temperature sensor abstraction with BME280 and DHT implementations
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

// Latest code available on https://github.com/framps/ESP_stuff/tree/main/ESPNow_DHT22_BME280_Sensor

#pragma once

#include <Arduino.h>

	class Sensor {

  public:

    struct Data {
        float temp;							// temperatur
        float hum;							// humidity
    };

   struct Delays {
        int activation;		  		// ms to wait when sensor vcc was turned on
        int startup;						// ms to wait when sensor was started
    };

    Sensor(uint8_t powerPin, Sensor::Delays delays) : powerPin(powerPin), delays(delays), debug(false) {};
    virtual ~Sensor() { };

    virtual float temperature() { return this->data.temp; };
    virtual float humidity() { return this->data.hum; };
    virtual void enableDebug() {this->debug = true; };

    virtual int start();                              // rc 0 -> request failed
    virtual int stop();                               // rc 0 -> request failed
    virtual int startSensor() = 0;                    // rc 0 -> request failed
    virtual int poll() = 0;                           // rc 0 -> request failed
    virtual const char* name() const = 0;

	protected:
    Data data;
		uint8_t powerPin;						// pin used to turn on/off Vcc of sensor
    Delays delays;              // sensor activation delays
    bool debug;
    };

  #include <BME280I2C.h>
  #include <Wire.h>

  class BME280Sensor : public Sensor {

  #define BME_ACTIVATION_DELAY_DEFAULT 10
  #define BME_STARTUP_DELAY_DEFAULT 0

  public:

    BME280Sensor (uint8_t powerPin = 0, Sensor::Delays delays = Sensor::Delays{BME_ACTIVATION_DELAY_DEFAULT, BME_STARTUP_DELAY_DEFAULT});
    virtual ~BME280Sensor () { };

    int startSensor();                           // rc 0 -> request failed
    int poll();                                  // rc 0 -> request failed
    const char* name() const { return "BME280"; };

  private:
    BME280I2C bme;
    };

  #include "DHT.h"

  class DHT22Sensor : public Sensor {

  #define DHT_ACTIVATION_DELAY_DEFAULT 1000
  #define DHT_STARTUP_DELAY_DEFAULT 0

  public:

    DHT22Sensor(uint8_t pin, uint8_t powerPin = 0, Sensor::Delays delays = Sensor::Delays{DHT_ACTIVATION_DELAY_DEFAULT, DHT_STARTUP_DELAY_DEFAULT});

    virtual ~DHT22Sensor() { };

    int startSensor();                           // rc 0 -> request failed
    int poll();                                  // rc 0 -> request failed
    const char* name() const { return "DHT22"; };

  private:
    DHT dht;
    uint8_t pin;
};
