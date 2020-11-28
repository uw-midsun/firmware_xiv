""" This modules provides the Python implementation to exchange data through SPI and the CAN protocol"""

import math
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
 
    if isinstance(port, str):
        port = getattr(GpioPort, port.capitalize())

    if spi_port < 0 or port >= GpioPort.NUM_GPIO_PORTS:
        raise ValueError("Expected port between A and {}".format(
            chr(GpioPort.NUM_GPIO_PORTS + ord('A') - 1)
        ))
    if spi_mode < 0 or spi_mode > 3:
        raise ValueError("Expected mode between 0 and 3")

    if cs == None:
        cs_port = 0
        cs_pin = 0
        use_cs = 0
    else:
        cs_port = cs[0]
        cs_pin = cs[1] 
        use_cs = 1

    data1 = [
        (10, 1), # id
        (spi_port, 1), 
        (spi_mode, 1),
        (len(tx_bytes), 1), # tx_len
        (rx_len, 1),
        (cs_port, 1),
        (cs_pin, 1),
        (use_cs, 1)
    ]

    data2 = [
        (11, 1), # id
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
            for i in 7 - len(data): 
                data.append(0)

        can_util.send_message(
            babydriver_id=BabydriverMessageId.SPI_EXCHANGE_TX_DATA,
            data=data
        )

    count = rx_len
    rx_data = []
    for i in range(0, math.ceil(rx_len / 7)):
        rx_msg = can_util.next_message(babydriver_id=BabydriverMessageId.SPI_EXCHANGE_RX_DATA)
        data = rx_msg.data
        if count < 7:
            tmp_data = []
            for i in count:
                tmp_data.append(data[i+1])
            rx_data.append(tmp_data)
            break
        rx_data.append(data[1:])
        count -= 7

    status = can_util.next_message(babydriver_id=BabydriverMessageId.STATUS)
    if status.data[1] != 0:
        raise Exception("Received STATUS_CODE {}".format(status.data[1]))

    return rx_data


    

    

    