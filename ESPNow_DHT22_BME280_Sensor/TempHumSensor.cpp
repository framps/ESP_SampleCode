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

#include "TempHumSensor.h"

int Sensor::start() {

  Serial.printf("Starting %s with delays %d and %d\n",this->name(), this->delays.activation,this->delays.startup);

  if ( usePowerPin ) {
    Serial.printf("Vcc enabled %s on pin %d\n", this->name(), this->powerPin);
    pinMode(this->powerPin, OUTPUT);        
    digitalWrite(this->powerPin, HIGH);     // Vcc power on
  }
  if ( this->delays.activation > 0 ) {
    Serial.printf("Activation delay on %s of pin %d %d\n",this->name(), this->powerPin, this->delays.activation);
    delay(this->delays.activation);
  }    

  Serial.println("***");
  int rc = this->startSensor();

  if ( this->delays.activation > 0 ) {
    Serial.printf("Startup delay on %s of %d\n",this->name(), this->delays.startup);
    delay(this->delays.startup);
  }
  return rc;
}

int Sensor::stop() {
  if ( usePowerPin ) {
    Serial.printf("Vcc disabled %s on pin %d\n", this->name(), this->powerPin);
    digitalWrite(this->powerPin, LOW);      // Vcc power off 
  }
}

BME280Sensor::BME280Sensor(bool usePowerPin, uint8_t powerPin, Sensor::Delays delays) : bme(), Sensor(usePowerPin, powerPin, delays) {}

int BME280Sensor::startSensor() {

  Wire.begin();

  int rc = 3;
  while (!bme.begin())
  {
    Serial.printf("Could not find BME280 sensor. Retry count %d\n", rc--);
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

  polledData.temp = floor(temp + 0.05);
  polledData.hum = floor(hum + 0.05);
  return 1;
}

DHT22Sensor::DHT22Sensor(uint8_t pin, bool usePowerPin, uint8_t powerPin, Sensor::Delays delays) : dht(pin, DHT22), Sensor(usePowerPin, powerPin, delays) {}

int DHT22Sensor::startSensor() {
  Serial.printf("Starting %s\n",this->name());
  this->dht.begin();
  return 1;
}

int DHT22Sensor::poll(Sensor::Data& polledData) {  
  polledData.hum = floor(this->dht.readHumidity() + 0.05);
  polledData.temp = floor(this->dht.readTemperature() + 0.05);
  Serial.printf("Polled %s with  data %f and %f\n",this->name(), polledData.temp, polledData.hum);
  return 1;
}
