# Steering

## What is the purpose of the board?
The steering board allows the driver to control important features of the vehicle as listed below:
* Horn
* Radio
* Lights
* Regen braking
* Cruise control
* Left and right signal

If you've ever driven a car, you'll know that there is a steering stalk in the vehicle, normally behind the steering wheel.
The steering board needs to detect when the driver moves the steering stalk into a certain position and act accordingly.
A picture of the steering stalk can be seen here:
https://uwmidsun.atlassian.net/wiki/spaces/ELEC/pages/846266737/Steering%2BInterface%2BMS%2B14

## What are all the things that the firmware needs to do?
There are two different inputs we can recieve from the steering stalk; an analog or a digital signal

### Digital
We rely on a digital signal for the horn, radio, lights, regen braking, and cruise control.
Our steering board will be connected to the steering stalk. When the steering stalk is manipulated, the steering board will
detect this and give a signal to the controller board. Our firmware will detect this signal and send a CAN message to act accordingly.

### Analog
The steering board gets an analog value when either the left or right signal is activated. Again, the steering board will detect this and send a CAN message to the car so that the left or right signal will turn on.

## How does it work?
* main.c - Initializes CAN and continually checks for events in a while loop
* steering_can.c - Checks what event was raised and sends a CAN message accordingly
* steering_control.c - Initializes an ADC to continously get an analog signal from the control stalk and raises a steering event when needed
* steering_digital_input.c - Sets up interrupts on GPIO pins to get a digital signal from the control stalk and raise a steering event when needed
