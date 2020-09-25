"""This module sends a CAN message for babydriver"""

import can

def send_message(message, device_id=15, msg_id=63):
    """Send a CAN message."""
    # TODO(SOFT-317): dynamically change between can0 and vcan0
    bus = can.interface.Bus(bustype='socketcan', channel='can0', bitrate=500000)

    # our CAN system uses 6 bits of message ID, 1 bit for ACK/DATA, and 4 bits for device ID
    arbitration_id = (msg_id << 5) | device_id

    msg = can.Message(
        arbitration_id=arbitration_id,
        data=message,
        is_extended_id=False
    )

    try:
        bus.send(msg)
        print("Message sent on {}".format(bus.channel_info))
    except can.CanError:
        print("Message NOT sent")
