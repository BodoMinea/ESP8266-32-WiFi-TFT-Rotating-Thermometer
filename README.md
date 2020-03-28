# ESP8266-32-WiFi-TFT-Rotating-Thermometer

Arduino thermometer with TFT Screen, of which display modes can be switched by rotating it around. Inspired by the Ikea CLOCKIS which works like this:

![IKEA KLOCKIS](https://sociorocketnews.files.wordpress.com/2018/07/kaiten.gif "IKEA KLOCKIS")

This one works like this:

![Project](device.gif "Project")

You need the following components:
* ESP32 NodeMCU indoor thermo/humidity display
    - TFT LCD Display, I used a square 128x128 clone
    - DHT11 sensor (if it's the bare sensor and not the board, you'll also need a resistor)
    - 4 tilt switches
    - jumper wires
* ESP8266/ESP-12E NodeMCU outdoor temperature probe
    - DS18B20 Dallas One-Wire compatible weatherproof temperature sensor with probe
    - 4.7k Resistor to put between VCC and data lines of said sensor
    
You will also need a DarkSky account to create an API Key. If you want to log your sensor data, you will need an installation of Grafana.

![Schematic](schematic.png "Schematic")
