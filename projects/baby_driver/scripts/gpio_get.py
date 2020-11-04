"""Python implementation of the gpio_get function for Babydriver."""

import can_util
from gpio_port import GpioPort
from message_defs import BabydriverMessageId

def gpio_get(port, pin):
    '''
    Returns the state of the GPIO pin at the given port and pin number as a bool

    The port can be entered as either a string or int value (e.g. 'A' or 0).
    The pin is an int value
    '''

    if port < 0 or port >= GpioPort.NUM_GPIO_PORTS:
        raise ValueError("ERROR: invalid GPIO port")
    if pin < 0 or pin >= 16:
        raise ValueError("ERROR: invalid GPIO pin number")

    data = [port, pin]
    can_util.send_message(
        babydriver_id=BabydriverMessageId.GPIO_GET_COMMAND,
        data=data,
        channel=can_util.default_channel
    )

    gpio_data_msg = can_util.next_message(
        babydriver_id=BabydriverMessageId.GPIO_GET_DATA,
        channel=can_util.default_channel
    )

    status_msg = can_util.next_message(
        babydriver_id=BabydriverMessageId.GPIO_GET_DATA,
        channel=can_util.default_channel
    )

    if status_msg.msg.data[0] != BabydriverMessageId.STATUS:
        raise Exception("ERROR: never recieved status message")

    msg_data = gpio_data_msg.msg.data[0]

    gpio_pin_state = msg_data == 1

    return gpio_pin_state
