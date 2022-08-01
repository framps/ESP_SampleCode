# ESP_stuff

1. LCD_Dimmer: Dim an LCD with SW instead of a potentiometer. Does not work with I2C driven LCD :-(. May work with native LCDs.
2. BlinkNotification: Simple class which allows to create any LED blink patterns for status and error notifications if there is no other way to pass this information to the outside world (e.g. LCD) ([Demovideo](https://www.linux-tips-and-tricks.de/BlinkNotification.mp4))
3. ESPNow and DeepSleeep Sample Code
    1. ESPNow_DHT22_BME280_Sensor: Sensor sketch which uses either BME280 or DHT22 sensor and uses this sensor to publish the data via ESPnow and DeepSleep to a ESPNow gateway by using deep sleep to minimze power consumption. Vcc power for the sensor can be turned off to reduce power consumption during deep sleep period. Vcc activation and sensor startup delays are configurable.
    2. ESPNow_gateway: Sample sketch to receive ESPNow sensor data and forward the data to an MQTT broker
4. OWM_BMP280: Sample sketch which queries current Open Weather Map data (temperature, humiditiy and pressure) and measures temperature and pressure with a BMP280.
5. Dewpoint: Calculate dew point, absolute moisture & vapor pressure from temperature & humidity
6. UpdateOtaPOC: Proof of concept for an ESP image update strategy which uses the LastUpdate header field of the image to decide whether an update of the image should be done with OTA. 
