""" This modules provides the Python implementation to exchange data through the SPI protocol"""

import can_util
from gpio_port import GpioPort
from message_defs import BabydriverMessageId

GPIO_PINS_PER_PORT = 16

# pylint: disable=invalid-name
# pylint: disable=too-many-arguments
# pylint: disable=too-many-branches
# pylint: disable=too-many-locals
def spi_exchange(tx_bytes, rx_len, spi_port=1, spi_mode=0, baudrate=6000000, cs=None):
    """
    Sends data through SPI
        The tx_bytes is an int array
        The rx_len is a non-negative int.
        The spi_port is an int
        The spi_mode is a non-negative int.
        The baudrate is a non-negative int.
        The CS pin, if not None, is a tuple (port, pin), where pin is an
            int in [0, NUM_PINS_PER_PORT) and port is either an int in [0, NUM_GPIO_PORTS),
            or a string 'a' through 'f', case insensitive
    Args:
        tx_bytes: The bytes to TX.
        rx_len: Number of bytes to RX
        spi_port: port of the pin to perform SPI exchange on (1 or 2)
        spi_mode: SPI mode to use (0, 1, 2 or 3)
        baudrate: baudrate to use
        cs: chip select port and pin to use (defaults to (1, 1))
    Raises:
        AttributeError: if cs_port parameter is entered as a string and is invalid
        ValueError: if spi_port, spi_mode, rx_len, cs_port, cs_pin don't meet their requirements
        Exception: if a non-zero status code was received.
    """

    if spi_port not in (1, 2):
        raise ValueError("ERROR: Expected SPI port 1 or 2")
    if spi_mode not in (0, 1, 2, 3):
        raise ValueError("ERROR: Expected mode between 0 and 3")
    if not 0 <= len(tx_bytes) <= 256:
        raise ValueError("ERROR: numbers of tx_bytes must be between 0 and 256")
    if not 0 <= rx_len <= 256:
        raise ValueError("ERROR: rx_len must be a non-negative integer between 0 and 256")
    if baudrate < 0:
        raise ValueError("ERROR: baudrate must be a non-negative integer")

    cs_port = 0 if cs is None else cs[0]
    cs_pin = 0 if cs is None else cs[1]
    use_cs = 0 if cs is None else 1
    if isinstance(cs_port, str):
        cs_port = getattr(GpioPort, cs_port.capitalize())

    if cs_port < 0 or cs_port >= GpioPort.NUM_GPIO_PORTS:
        raise ValueError("ERROR: Expected CS port between A and {}".format(
            chr(GpioPort.NUM_GPIO_PORTS + ord('A') - 1)
        ))
    if cs_pin < 0 or cs_pin >= GPIO_PINS_PER_PORT:
        raise ValueError("ERROR: Expected CS pin between 0 and {}".format(
            GPIO_PINS_PER_PORT - 1))

    data1 = [
        (spi_port - 1, 1), # 0 for SPI_PORT 1, 1 for SPI_PORT_2
        (spi_mode, 1),
        (len(tx_bytes), 1),  # tx_len
        (rx_len, 1),
        (cs_port, 1),
        (cs_pin, 1),
        (use_cs, 1),
    ]

    data2 = [
        (baudrate, 4)
    ]

    can_util.send_message(
        babydriver_id=BabydriverMessageId.SPI_EXCHANGE_METADATA_1,
        data=can_util.can_pack(data1)
    )

    can_util.send_message(
        babydriver_id=BabydriverMessageId.SPI_EXCHANGE_METADATA_2,
        data=can_util.can_pack(data2)
    )

    # collect bytes into groups of 7
    chunks = [tx_bytes[x:x+7] for x in range(0, len(tx_bytes), 7)]

    for data in chunks:
        # pad with 0s if length isn't 7
        if len(data) < 7:
            data += [0] * (7 - len(data))
        tmp = [(d, 1) for d in data]
        can_util.send_message(
            babydriver_id=BabydriverMessageId.SPI_EXCHANGE_TX_DATA,
            data=can_util.can_pack(tmp)
        )

    # Receive bytes
    rx_bytes_left = rx_len
    rx_data = []
    while rx_bytes_left > 0:
        rx_msg = can_util.next_message(
            babydriver_id=BabydriverMessageId.SPI_EXCHANGE_RX_DATA)
        msg_data = rx_msg.data
        rx_data += list(msg_data[1:min(rx_bytes_left+1, 8)])
        rx_bytes_left -= 7

    status = can_util.next_message(babydriver_id=BabydriverMessageId.STATUS)

    if status.data[1] != 0:
        raise Exception(
            "ERROR: Received non-zero status code: {}".format(status.data[1]))

    return rx_data
