Adafruit_DotStar
================
Added ATtiny84a support which can be used for the PCB wearable:
https://hackaday.io/project/162435-board-for-modular-led-wearables

See example "RainbowStar" for multi-strip output with multiple data lines.
Example also shows the use of millis() instead of delays() so that the main loop can be used to read input pins.
