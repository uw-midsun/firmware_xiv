# Motor Controller Interface

## What is the purpose of the board?
The motor controllers (MC) are a third party board with a custom CAN communication protocol. Currently, we use
WaveSculptor 22. The MC must send and receive messages to the driver controls (DC), however because the MC protocol is strict and thorough it requires abstraction from the rest of the system. The motor controller interface (MCI) handles this abstraction.

The MC connects to its own CAN bus in order to communicate with the MCI. The MCI connects to the system CAN bus in order to communicate with DC.

https://uwmidsun.atlassian.net/wiki/spaces/ELEC/pages/845906388/Motor%2BController%2BInterface%2BMS%2B14

## How does it work?
* cruise_rx - Handles cruise control velocity
* drive_fsm - Handles the FSM to transition the car between drive, reverse, and neutral. It also handles cruise control ON/OFF
* main - Calls functions from all modules in this project
* mci_broadcast - Broadcasts data from the MC
* mci_output - Outputs messages to MC
* motor_can - Packs CAN messages for the MC
* precharge_control - Handles the precharge which limits the inrush current when we power on
