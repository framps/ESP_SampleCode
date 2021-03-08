#pragma once

	class Sensor {

  public:

    struct Data {
        float temp;
        float hum;
    };

    Sensor() {};      
    virtual ~Sensor() { };
      
    virtual int start() = 0;                          // rc 0 -> OK, fail otherwise    
    virtual int poll(Data& polledData) = 0;           // rc 0 -> OK, fail otherwise

    };

  #include "DHT.h"
  
	class DHT22Sensor : public Sensor {

  public:

    DHT22Sensor();      
    virtual ~DHT22Sensor() { };
      
    int start();                                 // rc 0 -> OK, fail otherwise    
    int poll(Sensor::Data& polledData);          // rc 0 -> OK, fail otherwise

  private:
    DHT dht;
    };


  #include <BME280I2C.h>
  #include <Wire.h>

  class BME280Sensor : public Sensor {

  public:

    BME280Sensor ();      
    virtual ~BME280Sensor () { };
      
    int start();                                 // rc 0 -> OK, fail otherwise    
    int poll(Sensor::Data& polledData);          // rc 0 -> OK, fail otherwise

  private:
    BME280I2C bme;
    };
