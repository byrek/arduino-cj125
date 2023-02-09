# Required Arduino libraries:
- SPI
- PID_v1

#Installation:
Create a CJ125 directory under Your Arduino libraries directory.

Drop the content in there & have fun!

# Bosch LSU4.9 5-wire identification:
- IP      red             (czerwony)
- VM      yellow          (żółty)
- HG      white           (biały)
- B+      grey            (szary)
- IA      not connected   (nie podłączony)
- UN      black           (czarny)

# Arduino control lines

|Arduino line|Shield|
|------------------|:--------------------------:|
|RESET|CJ125 Reset line|
|5V|5V|
|GND|GND|
|VIN|When JP1 is closed, the arduino is supplied with +/- BAT terminals|
|Analog in 0|CJ125 UA|
|Analog in 1|CJ125 UR|
|Analog in 3|Battery voltage monitoring|
|4|Analog output option (might be used to deliver O2 narrow band signal, not implemented in code)|
|6|PWM output for LSU heater|
|19|SPI CJ125 Chip select line (NSS)|
|11|SPI MOSI|
|12|SPI MISO|
|13|SPI CLK|
