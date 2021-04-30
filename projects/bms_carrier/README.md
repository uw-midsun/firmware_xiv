# BMS Carrier

## What is the purpose of the board?
The BMS carrier board ensures that the batteries in our solar car operate properly and do not explode.
We ensure our batteries operate properly by:
* Monitoring data such as voltage, temperature, current, and the state of our relays
* Disconnecting our relays if we notice undervoltage
* Cooling our batteries with fans depending on how hot the batteries are

## What are all the things that the firmware needs to do?
* Relay control: to close relays, pull the pin high and verify successful. There are two relays to control.
* Killswitch: monitors killswitch status, if pressed, fault BPS
* Current sense: talk to the ADS1259 through SPI, periodically read current, and fault BPS if overcurrent
* Voltage sense: talk to daisy chained LTC6811s through SPI, trigger ADC conversions, read back the results, fault BPS if overvoltage or undervoltage
* Temperature sense: cycle through 20 thermistors through the ADG731 MUX, trigger conversions, read back the results, fault BPS if overtemp
* BPS heartbeat: periodically send a heartbeat to rest of car, ensure it’s acknowledged
* Fan control: control 4 fans, notify if they’re stuck
* Logging: log all temperature, voltage, current, fan state, and relay state data over CAN.
* Passive balancing: tell the AFEs to bleed the highest charge cell until all cells are within 25mV.

## How does it work?
More details can be found at https://uwmidsun.atlassian.net/wiki/spaces/ELEC/pages/850199346/BMS+Carrier+Firmware
There is a block diagram which describes how each part above interacts with each other.
