"""This module provides the Python implementation to set GPIO pins using the CAN protocol"""

import can_util
from gpio_port import GpioPort
from message_defs import BabydriverMessageId

NUM_PINS_PER_PORT = 16


def gpio_set(port, pin, state):
    """
    Sets a GPIO pin
    Args:
        port: port of the GPIO pin to set (can be entered as either a string or integer)
        pin: pin number of the GPIO pin to set
        state: 0 if setting low, 1 if setting high
    Raises:
        AttributeError: if port parameter is entered as a string and is invalid
        ValueError: if parameters passed into gpio_set are invalid
        Exception: if a non-zero status code was received
    """

    # If given port as a string, it is converted to a string
    if isinstance(port, str):
        port = getattr(GpioPort, port.capitalize())

    # Checks whether valid parameters were passed into gpio_set
    if port < 0 or port >= GpioPort.NUM_GPIO_PORTS:
        raise ValueError("Expected port between A and {}".format(
            chr(GpioPort.NUM_GPIO_PORTS + ord('A') - 1)
        ))
    if pin < 0 or pin >= NUM_PINS_PER_PORT:
        raise ValueError("Expected pin between 0 and {}".format(NUM_PINS_PER_PORT - 1))
    if state not in (0, 1):
        raise ValueError("Expected state of 0 (low) or 1 (high)")

    can_pack_args = [(port, 1), (pin, 1), (state, 1)]

    can_util.send_message(
        babydriver_id=BabydriverMessageId.GPIO_SET,
        data=can_util.can_pack(can_pack_args)
    )

    gpio_set_status = can_util.next_message(babydriver_id=BabydriverMessageId.STATUS)

    # Checks for valid status code
    if gpio_set_status.data[1] != 0:
        raise Exception("Received STATUS_CODE {}".format(gpio_set_status.data[1]))
