# CAN dump

## Setup

You may wish to setup `udev` rules in order to make finding the Xbee device 
easier.

Under `/etc/udev/rules.d/`, create a file which will contain the `udev` rule, 
called `/etc/udev/rules.d/10-xbee.rules`. Then add the following:

```
SUBSYSTEM=="tty", ATTRS{idVendor}=="0403", ATTRS{idProduct}=="6015", SYMLINK+="ttyXbee", MODE="0666"
```

## Usage

```bash
# Install requirements.txt (from a virtualenv or something)
pip install -r requirements.txt

# Run the script from the root of the firmware repository
python projects/can_dump/scripts/dump_system.py /dev/ttyXbee

# Messages can be filtered by CAN id using the -m flag
python projects/can_dump/scripts/dump_system.py /dev/ttyXbee -m32

# The log data can be parsed using the parse_data.py script
python projects/can_dump/script/parse_data.py --file='logs/system_can_2018-07-20 08:12:22.467099.log'
```
