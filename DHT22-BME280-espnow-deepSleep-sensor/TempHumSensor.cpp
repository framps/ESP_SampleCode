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

#include "TempHumSensor.h"

DHT22Sensor::DHT22Sensor(uint8_t pin, uint8_t type, uint8_t powerPin) : dht(pin, type, powerPin), powerPin(powerPin) {
}

int DHT22Sensor::start() {

  pinMode(this->powerPin,OUTPUT);     //DHT POWER PIN +5V

  Serial.println("Enable DHT");
  digitalWrite(this->powerPin,HIGH);

  dht.begin();
  delay(1000);                       // wait for DHT22 to start up

  return 1;
}

int DHT22Sensor::poll(Sensor::Data& polledData) {
  
  polledData.temp=floor(this->dht.readHumidity()+0.05);
  polledData.hum=floor(this->dht.readTemperature()+0.05);
  
  return 1;
}

BME280Sensor::BME280Sensor(uint8_t powerPin) : bme(), powerPin(powerPin) {
}

int BME280Sensor::start() {

  pinMode(this->powerPin,OUTPUT);     // BME POWER PIN +5V

  Serial.println("Enable BME");
  digitalWrite(this->powerPin,HIGH);

  Wire.begin();

  int rc=3;
  while(!bme.begin())
  {
    Serial.printf("Could not find BME280 sensor. Retry count %d\n",rc--);
    delay(1000);
    if (rc <= 0) {
      return 0;
    };
  }
  return 1;
}

int BME280Sensor::poll(Sensor::Data& polledData) {

  float temp(NAN), hum(NAN), pres(NAN);

  BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
  BME280::PresUnit presUnit(BME280::PresUnit_Pa);
  
  this->bme.read(pres, temp, hum, tempUnit, presUnit);
  
  polledData.temp=floor(temp+0.05);
  polledData.hum=floor(hum+0.05);
  return 1;
}
