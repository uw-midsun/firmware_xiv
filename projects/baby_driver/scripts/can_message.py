#!/usr/bin/env python
# coding: utf-8
from __future__ import print_function

import can

def send_message(msg):
    """sends can message with 8 bytes. You can change the 2nd one"""
    #change the channel later to can0 or vcan0
    bus = can.interface.Bus(bustype='socketcan', channel='can0', bitrate=250000)

    msg = can.Message(
        arbitration_id=0xC0FFEE, data=[0, msg, 0, 0, 0, 0, 0, 0], is_extended_id=False
    )

    try:
        bus.send(msg)
        print("Message sent on {}".format(bus.channel_info))
    except can.CanError:
        print("Message NOT sent")
