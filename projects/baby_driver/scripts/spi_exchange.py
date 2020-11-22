""" This modules provides the Python implementation to exchange data through SPI and the CAN protocol"""

import can_util
from gpio_port import GpioPort
from message_defs import BabydriverMessageId

def spi_exchange(tx_bytes, rx_len, spi_port=1, spi_mode=0, baudrate=6000000, cs=None):
    """
    Sends data through SPI
    Args: 
        tx_bytes:
        rx_len:
        spi_port:
        spi_mode:
        baudrate:
        cs: 
    Raises: 
        AttributeError: if port parameter is entered as a string and is invalid 
    """

    assert (spi_port == 1) or (spi_port == 2)
    assert (spi_mode >= 0) and (spi_mode <= 3)
 
    if isinstance(port, str):
        port = getattr(GpioPort, port.capitalize())

    if port < 0 or port >= GpioPort.NUM_GPIO_PORTS:
        raise ValueError("Expected port between A and {}".format(
            chr(GpioPort.NUM_GPIO_PORTS + ord('A') - 1)
        ))

    

    