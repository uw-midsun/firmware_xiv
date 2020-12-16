""" This modules provides the Python implementation to exchange data through SPI and the CAN protocol"""

import math
import can_util
from gpio_port import GpioPort
from message_defs import BabydriverMessageId

def spi_exchange(tx_bytes, rx_len, spi_port=1, spi_mode=0, baudrate=6000000, cs=None):
    """
    Sends data through SPI

    The tx_bytes is an int array
    The rx_len is an int.
    The spi_port can be entered as either a string or int value (e.g. 'A' or 0). 
        Must be pin 1 or 2
    The spi_mode is an int
    The baudrate is an int
    The pin is an int value
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
        AttributeError: if port parameter is entered as a string and is invalid 
    """

    if spi_port < 0 or spi_port >= GpioPort.NUM_GPIO_PORTS:
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

    if isinstance(cs_port, str):
        cs_port = getattr(GpioPort, cs_port.capitalize())

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

    # print(data1)
    # print(data2)

    # print("tx")
    # print(tx_bytes)

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
    # TODO turn byte arr into int arr
    chunks = [tx_bytes[x:x+7] for x in range(0, len(tx_bytes), 7)] 

    # print("chunks")
    # print(chunks)

    for data in chunks: 
        # pad with 0s if length isn't 7
        if len(data) < 7:
            # narr = data
            # print(barr)
            for i in range(7 - len(data)): 
                data.append(0)
            # print(barr)   
            # data = narr 
        # print(data)
        tmp = [12]
        tmp.extend(data)
        # tmp = can_util.can_pack(tmp)
        tmp = [
            (tmp[0], 1),
            (tmp[1], 1),
            (tmp[2], 1),
            (tmp[3], 1),
            (tmp[4], 1),
            (tmp[5], 1),
            (tmp[6], 1),
            (tmp[7], 1)
        ]
        # print(tmp)
        new_data = can_util.can_pack(tmp)
        print(new_data)
        can_util.send_message(
            babydriver_id=BabydriverMessageId.SPI_EXCHANGE_TX_DATA,
            data=new_data
        )

    # print(chunks)

    # Receive bytes

    count = rx_len
    rx_data = []
    print(range(0, math.ceil(rx_len / 7) + 1))
    for i in range(0, math.ceil(rx_len / 7) + 1):
        rx_msg = can_util.next_message(
            babydriver_id=BabydriverMessageId.SPI_EXCHANGE_RX_DATA)
        data = rx_msg.data[1]
        print("data {}".format(i))
        print(rx_msg)
        print(data)
        if count < 7:
            tmp_data = []
            for i in range(0, count):
                # print(data[i+1])
                tmp_data.append(data[i+1])
            rx_data.append(tmp_data)
            break
        rx_data.append(data[1:])
        count -= 7

    status = can_util.next_message(babydriver_id=BabydriverMessageId.STATUS)

    if status.data[1] != 0:
        # print("rx_data")
        # print(rx_data)
        raise Exception("Received STATUS_CODE {}".format(status.data[1]))
        # raise Exception("ERROR: Non-OK status returned: {}".format(status))

    return rx_data


    

    

    