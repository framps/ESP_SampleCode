#include "./TempHumSensor.h"

Sensor *s;

void setup() {

    Serial.begin(115200);
    Serial.println("... setup ...");

    BME280Sensor *bs = new BME280Sensor;

    if (bs->start() == 0) {
        Serial.println("Using BME280");
        s = bs;
    }
    
    else {

      delete bs;
      bs = NULL;

      DHT22Sensor *ds = new DHT22Sensor;

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
