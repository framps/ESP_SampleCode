#include "TempHumSensor.h"

#define DHTPIN 12 // D2 is GPIO 4
#define DHTPower 15 // D8
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

DHT22Sensor::DHT22Sensor() : dht(DHTPIN, DHTTYPE) {
}

int DHT22Sensor::start() {

  pinMode(DHTPower,OUTPUT);     //DHT POWER PIN +5V

  Serial.println("Enable DHT");
  digitalWrite(DHTPower,HIGH);

  delay(10);
  dht.begin();

  return 0;
}

int DHT22Sensor::poll(Sensor::Data& polledData) {
  
  polledData.temp=floor(this->dht.readHumidity()+0.05);
  polledData.hum=floor(this->dht.readTemperature()+0.05);
  
  return 0;
}

#define BMEPower 15 // D8

BME280Sensor::BME280Sensor() : bme() {
}

int BME280Sensor::start() {

  pinMode(BMEPower,OUTPUT);     // BME POWER PIN +5V

  Serial.println("Enable BME");
  digitalWrite(BMEPower,HIGH);

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
