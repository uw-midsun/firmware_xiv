"""Python implementation of the gpio_get function for Babydriver."""

import can_util
from gpio_port import GpioPort
from message_defs import BabydriverMessageId

GPIO_PINS_PER_PORT = 16


def gpio_get(port, pin):
    '''
    Returns the state of the GPIO pin at the given port and pin number as a bool

    The port can be entered as either a string or int value (e.g. 'A' or 0).
    The pin is an int value
    '''

    if isinstance(port, str):
        port = getattr(GpioPort, port.capitalize())

    if port < 0 or port >= GpioPort.NUM_GPIO_PORTS:
        raise ValueError("ERROR: invalid GPIO port")
    if pin < 0 or pin >= GPIO_PINS_PER_PORT:
        raise ValueError("ERROR: invalid GPIO pin number")

    data = can_util.can_pack([(port, 1), (pin, 1)])

    can_util.send_message(
        babydriver_id=BabydriverMessageId.GPIO_GET_COMMAND,
        data=data,
    )

    gpio_data_msg = can_util.next_message(
        babydriver_id=BabydriverMessageId.GPIO_GET_DATA,
    )

    status_msg = can_util.next_message(
        babydriver_id=BabydriverMessageId.STATUS,
    )

    status = status_msg.data[1]
    if status != 0:
        raise Exception("ERROR: Non-OK status returned: {}".format(status))

    raw_state = gpio_data_msg.data[1]

    gpio_pin_state = bool(raw_state)

    return gpio_pin_state
