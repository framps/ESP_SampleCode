# ESP_stuff

1. LCD_Dimmer: Dim an LCD with SW instead of a potentiometer. Does not work with I2C driven LCD :-(. May work with native LCDs.
2. BlinkNotification: Simple class which allows to create any LED blink patterns for status and error notifications if there is no other way to pass this information to the outside world (e.g. LCD) ([Demovideo](https://www.linux-tips-and-tricks.de/BlinkNotification.mp4))
3. DHT22-BME280-espnow-deepSleep-sensor: Sensor sketch which detects BME280 and DHT22 sensor and uses this sensor to publish the date via ESPnow to a MQTT broker. If there is a LCD display connected the display will be used to display measured temperature and humidity and current time and date - NOTE: Development in progress
