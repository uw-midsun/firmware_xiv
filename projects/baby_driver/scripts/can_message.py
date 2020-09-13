#!/usr/bin/env python
# coding: utf-8
from __future__ import print_function

import can

def send_message(msg):
    bus = can.interface.Bus(bustype='socketcan', channel='vcan0', bitrate=250000)

    msg = can.Message(
        arbitration_id=0xC0FFEE, data=[0, msg, 0, 0, 0, 0, 0, 0], is_extended_id=False
    )

    try:
        bus.send(msg)
        print("Message sent on {}".format(bus.channel_info))
    except can.CanError:
        print("Message NOT sent")
