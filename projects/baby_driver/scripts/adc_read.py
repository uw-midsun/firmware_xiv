"""Python implementation of the adc_read function for Babydriver."""

import can_util
from gpio_port import GpioPort
from message_defs import BabydriverMessageId

NUM_PINS_PER_PORT = 16
OK_STATUS = 0

def adc_read(port, pin, raw):
    """
    Reads a raw or converted ADC value.

    Args:
        port: The port of the GPIO pin to read from.
        pin: The pin number of the GPIO pin to read from.
        raw: If raw is True, a raw read should be performed; otherwise a converted read.

    Returns:
        The ADC reading as a 16-bit integer.

    Raises:
        ValueError: if the range of the input args is invalid.
        Exception: if we receive a nonzero status code.
    """

    # If port is entered as a str, convert to int
    if isinstance(port, str):
        port = getattr(GpioPort, port.capitalize())

    # Check the ranges of input args
    if port < 0 or port >= GpioPort.NUM_GPIO_PORTS:
        raise ValueError("ERROR: invalid GPIO port")
    if pin < 0 or pin >= NUM_PINS_PER_PORT:
        raise ValueError("ERROR: invalid GPIO pin number")
    if raw not in (True, False):
        raise ValueError("ERROR: raw must be a bool value")

    # Send CAN message
    data = [
        (port, 1),
        (pin, 1),
        (raw, 1),
    ]
    data = can_util.can_pack(data)
    can_util.send_message(data)

    # Wait to receive the first CAN message with data
    data_msg = can_util.next_message(babydriver_id=BabydriverMessageId.ADC_READ_COMMAND)

    # Extract the low and high bytes of the ADC conversion
    result_low = data_msg.data[1]
    result_high = data_msg.data[2]

    # Wait to receive the second CAN message with status
    status_msg = can_util.next_message(babydriver_id=BabydriverMessageId.STATUS)
    status = status_msg.data[1]

    # Check if status is not ok
    if status != OK_STATUS:
        raise Exception("ERROR: received a nonzero STATUS_CODE: {}".format(status))

    return (result_high << 8) | result_low
