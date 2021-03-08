#include "TempHumSensor.h"

Sensor *s;

#define DHTPIN 12               // D2 is GPIO 4
#define DHTPower 15             // Vcc power pin D8
#define DHTTYPE DHT22           // DHT 22  (AM2302), AM2321
#define BMEPower DHTPower       // Vcc power pin D8 

void setup() {

    Serial.begin(115200);
    Serial.println("... setup ...");

    // Check whether BMD280 sensor is available

    BME280Sensor *bs = new BME280Sensor(BMEPower);

    if (bs->start() == 0) {
        Serial.println("Using BME280");
        s = bs;
    }
    
    else {

      delete bs;
      bs = NULL;

      // Fall back to DHT22 sensor

      DHT22Sensor *ds = new DHT22Sensor(DHTPIN, DHTTYPE, DHTPower);

      if (ds->start() == 0) {
          Serial.println("Using DHT22");
          s = ds;
      }
      else {
          Serial.println("Initialization failed");
          delay(3000);
          ESP.restart();
      }
   }
}

void loop() {

    Serial.println("... loop ...");

    Sensor::Data sensorData;

    if (s->poll(sensorData) != 0) {
        Serial.println("Poll failed");
        delay(3000);
        ESP.restart();
    };

    Serial.printf("Temp: %f, Hum: %f\n",sensorData.temp, sensorData.hum);
    delay(3000);
}
