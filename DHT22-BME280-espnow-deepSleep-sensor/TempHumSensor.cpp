#include "TempHumSensor.h"

DHT22Sensor::DHT22Sensor(uint8_t pin, uint8_t type, uint8_t powerPin) : dht(pin, type, powerPin), powerPin(powerPin) {
}

int DHT22Sensor::start() {

  pinMode(this->powerPin,OUTPUT);     //DHT POWER PIN +5V

  Serial.println("Enable DHT");
  digitalWrite(this->powerPin,HIGH);

  delay(10);
  dht.begin();

  return 0;
}

int DHT22Sensor::poll(Sensor::Data& polledData) {
  
  polledData.temp=floor(this->dht.readHumidity()+0.05);
  polledData.hum=floor(this->dht.readTemperature()+0.05);
  
  return 0;
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
      return 1;
    };
  }
  return 0;
}

int BME280Sensor::poll(Sensor::Data& polledData) {

  float temp(NAN), hum(NAN), pres(NAN);

  BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
  BME280::PresUnit presUnit(BME280::PresUnit_Pa);
  
  this->bme.read(pres, temp, hum, tempUnit, presUnit);
  
  polledData.temp=floor(temp+0.05);
  polledData.hum=floor(hum+0.05);
  return 0;
}
