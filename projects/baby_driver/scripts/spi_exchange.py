""" This modules provides the Python implementation to exchange data through the SPI protocol"""

import math
import sys
import can_util
from gpio_port import GpioPort
from message_defs import BabydriverMessageId

GPIO_PINS_PER_PORT = 16

#TODO add error in front of error messages

def spi_exchange(tx_bytes, rx_len, spi_port=1, spi_mode=0, baudrate=6000000, cs=None):
    """
    Sends data through SPI

        The tx_bytes is an int array
        The rx_len is a non-negative int.
        The spi_port can be entered as either a string or int value (e.g. 'A' or 0). 
        The spi_mode is an int
        The baudrate is an int
        The CS pin, if not None, should be passed as a tuple (port, pin), where pin is an
            int in [0, NUM_PINS_PER_PORT) and port is either an int in [0, NUM_GPIO_PORTS),
            or a string 'a' through 'f', case insensitive;
    
    Args: 
        TODO 
        tx_bytes: The bytes to TX.
        rx_len: Number of bytes to RX
        spi_port: port of the GPIO pin to perform SPI exchange on (1 or 2)
        spi_mode: SPI mode to use (0, 1, 2 or 3)
        baudrate:
        cs: 
    Raises: 
        TODO 
        AttributeError: if spi_port parameter is entered as a string and is invalid 
    """

    # If port is entered as a str, convert to int
    if isinstance(spi_port, str):
        spi_port = getattr(GpioPort, spi_port.capitalize())

    if spi_port < 1 or spi_port > 2:
        raise ValueError("ERROR: Expected SPI port A or B")
    if spi_mode < 0 or spi_mode > 3:
        raise ValueError("ERROR: Expected mode between 0 and 3")
    if rx_len < 0:
        raise ValueError("ERROR: rx_len must be a non-negative integer")

    if cs == None:
        cs_port = 0
        cs_pin = 0
        use_cs = 0
    else:
        cs_port = cs[0]
        cs_pin = cs[1] 
        use_cs = 1

    if isinstance(cs_port, str):
        cs_port = getattr(GpioPort, cs_port.capitalize())

    if cs_port < 0 or cs_port >= GpioPort.NUM_GPIO_PORTS:
        raise ValueError("ERROR: invalid CS GPIO port")
    if cs_pin < 0 or cs_pin >= GPIO_PINS_PER_PORT:
        raise ValueError("ERROR: invalid CS GPIO pin number")

    data1 = [
        (spi_port, 1), 
        (spi_mode, 1),
        (len(tx_bytes), 1), # tx_len
        (rx_len, 1),
        (cs_port, 1),
        (cs_pin, 1),
        (use_cs, 1)
    ]

    data2 = [
        (baudrate, 4)
    ]

    data1 = can_util.can_pack(data1)
    data2 = can_util.can_pack(data2)

    can_util.send_message(
        babydriver_id=BabydriverMessageId.SPI_EXCHANGE_METADATA_1,
        data=data1
    )

    can_util.send_message(
        babydriver_id=BabydriverMessageId.SPI_EXCHANGE_METADATA_2,
        data=data2
    )

    # collect bytes into groups of 7
    chunks = [tx_bytes[x:x+7] for x in range(0, len(tx_bytes), 7)] 

    for data in chunks: 
        # pad with 0s if length isn't 7
        if len(data) < 7:
            for i in range(7 - len(data)): 
                data.append(0)
        tmp = [
            (data[0], 1),
            (data[1], 1),
            (data[2], 1),
            (data[3], 1),
            (data[4], 1),
            (data[5], 1),
            (data[6], 1),
        ]
        can_util.send_message(
            babydriver_id=BabydriverMessageId.SPI_EXCHANGE_TX_DATA,
            data=can_util.can_pack(tmp)
        )

    # Receive bytes
    count = rx_len
    rx_data = []
    for i in range(0, math.ceil(rx_len / 7) + 1):
        rx_msg = can_util.next_message(
            babydriver_id=BabydriverMessageId.SPI_EXCHANGE_RX_DATA)
        d = rx_msg.data
        if count <= 7:
            tmp_data = []
            for i in range(0, count):
                tmp_data.append(d[i])
            rx_data = rx_data + tmp_data
            break
        rx_data = rx_data + d[0:]
        count -= 7

    status = can_util.next_message(babydriver_id=BabydriverMessageId.STATUS)

    if status.data[1] != 0:
        raise Exception("ERROR: Received non-zero status code: {}".format(status.data[1]))

    return rx_data


    

    

    