""" This module sends a CAN message for babydriver"""

import can

def send_message(message):
    """sends can message with 8 bytes. You can change the 2nd one"""
    # change the channel later to can0 or vcan0
    bus = can.interface.Bus(bustype='socketcan', channel='can0', bitrate=500000)

    msg = can.Message(
        arbitration_id=0x2,
        data=message,
        is_extended_id=False
    )

    try:
        bus.send(msg)
        print("Message sent on {}".format(bus.channel_info))
    except can.CanError:
        print("Message NOT sent")
