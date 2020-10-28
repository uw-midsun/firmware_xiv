"""This module provides the Python implementation to set GPIO pins using the CAN protocol"""

import can_util

def gpio_set(port, pin, state):
    """
    Sets a GPIO pin
    Args:
        port: port of the GPIO pin to set
        pin: pin number of the GPIO pin to set
        state: 0 if setting low, 1 if setting high
    Returns:
        A boolean value. If true, the CAN message was sent successfully, otherwise
        a ValueError or TimeoutError was raised.
    """
    if 0 <= port <= 6 and 0 <= pin <= 15 and 0 <= state <= 1:
        can_pack_args = [(port, 1), (pin, 1), (state, 1)]
    else:
        return False

    try:
        can_util.send_message(babydriver_id=1, data=can_util.can_pack(can_pack_args))
    except ValueError:
        return False

    try:
        can_util.next_message(babydriver_id=1)
        return True
    except ValueError:
        return False
    except TimeoutError:
        return False
