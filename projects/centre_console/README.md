# Centre console

## What is the purpose of the board?
The centre console is responsible for:
* Handling the physical button interface so the driver can change from neutral, reverse, and drive
* Powers the Raspberry Pi responsible for the driver display
* Controls fans to cool the Raspberry Pi
* Toggles the BPS indicator
* Controls the hazards

https://uwmidsun.atlassian.net/l/c/bG021psD

## What are all the things that the firmware needs to do?
Here are the following tasks:
* Detect button presses and use our FSM to change states accordingly (eg if the driver presses neutral the car should go into neutral)
* Control LEDs on the centre console to display BPS faults and also the current state the car is in
* Detect if the hazard button is pressed and enable the hazards
* Organize how the centre console, along with the car, are powered on and off
* Create monitors for faults, pedal, precharge, and speed

## How does it work?
* button_press - Checks for button presses
* charging_manager - Checks if the car is charging properly
* drive_fsm - FSM to handle drive states
* fault_monitor - Faults when BPS heartbeat fails
* hazard_tx - Controls the hazards
* led_manager - Controls LEDs that help indicate what state the car is in
* main_event_generator - Generates events for changes in drive state
* main - Initializes and runs all the other source files
* mci_output_tx - Ensures MCI fully transitions between different drive states
* pedal_monitor - This module uses pedal_rx to find the state of the pedal
* power_aux_sequence - A simple state machine to ensure the car can power off properly from the aux battery
* power_fsm - A high level FSM taht covers power_main_sequence, power_off_sequence, and power_aux_sequence
* power_main_sequence - Helps ensure all steps to power the car off from the main battery are done successfully
* power_off_sequence - Controls power off sequence
* precharge_monitor - Waits for precharge to complete and times out after a couple of seconds.
* race_switch - FSM to switch between race mode and normal mode
* speed_monitor - Monitors the speed
