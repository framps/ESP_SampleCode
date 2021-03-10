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

	class Sensor {

  public:

    struct Data {
        float temp;
        float hum;
    };

    Sensor() {};      
    virtual ~Sensor() { };
      
    virtual int start() = 0;                          // rc 0 -> request failed
    virtual int poll(Data& polledData) = 0;           // rc 0 -> request failed
    virtual const char* name() = 0;
    };

  #include "DHT.h"
  
	class DHT22Sensor : public Sensor {

  public:

    DHT22Sensor(uint8_t pin, uint8_t type, uint8_t powerPin);
     
    virtual ~DHT22Sensor() { };
      
    int start();                                 // rc 0 -> request failed   
    int poll(Sensor::Data& polledData);          // rc 0 -> request failed
    const char* name() { return "DHT22"; };

  private:
    DHT dht;
    uint8_t powerPin;
    };


  #include <BME280I2C.h>
  #include <Wire.h>

  class BME280Sensor : public Sensor {

  public:

    BME280Sensor (uint8_t powerPin);      
    virtual ~BME280Sensor () { };
      
    int start();                                 // rc 0 -> request failed
    int poll(Sensor::Data& polledData);          // rc 0 -> request failed
    const char* name() { return "BME280"; };

  private:
    BME280I2C bme;
    uint8_t powerPin;
    };
