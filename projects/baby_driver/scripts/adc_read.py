"""Python implementation of the adc_read function for Babydriver."""

import can_util
from gpio_port import GpioPort
from message_defs import BabydriverMessageId

NUM_PINS_PER_PORT = 16

def adc_read(port, pin, raw):
    """
    Returns a raw or converted ADC reading on a specific port and pin from the firmware project.

    The port can be entered as either a string or int value (e.g. 'A' or 0).
    The pin is an int value, and raw is a bool value.
    """

    # If port is entered as a str, convert to int
    if isinstance(port, str):
        port = getattr(GpioPort, port.capitalize())

    # Check the ranges of input args
    if port < 0 or port >= GpioPort.NUM_GPIO_PORTS: 
        raise ValueError("ERROR: invalid GPIO port")
    if pin < 0 or pin >= NUM_PINS_PER_PORT: 
        raise ValueError("ERROR: invalid GPIO pin number")
    if raw != True and raw != False:
        raise ValueError("ERROR: raw must be a bool value")

    # Send CAN message
    data = [
        (BabydriverMessageId.ADC_READ_COMMAND, 1),
        (port, 1),
        (pin, 1),
        (raw, 1),
    ]
    data = can_util.can_pack(data)
    can_util.send_message(data)

    # Wait to receive the first CAN message with data
    data_mssg = can_util.next_message()

    # Check if received message is an adc_read
    if data_mssg.data[0] != BabydriverMessageId.ADC_READ_DATA:
        raise Exception("ERROR: did not receive adc read data")

    # Extract the low and high bytes of the ADC conversion
    result_low = data_mssg.data[1]
    result_high = data_mssg.data[2]

    # Wait to receive the second CAN message with status
    status_mssg = can_util.next_message()

    # Check if status is not ok (if it's nonzero)
    if status_mssg.data[1] != BabydriverMessageId.STATUS:
        raise Exception("ERROR: received a nonzero STATUS_CODE: {}".format(status_mssg.data[1]))

    # Return ADC reading as a 16-bit int
    return (result_high << 8) | result_low
