"""Python implementation of the i2c_read function for Babydriver."""

import can_util
from math import ceil
from message_defs import BabydriverMessageId

def i2c_read(port, address, rx_len, reg=None):
    """
    Reads a i2c 
    Args:
        port: I2C port (0 or 1 for I2C_PORT_1 or I2C_PORT_2) to read from
        address: I2C address to read from
        rx_len: expected response length
        reg: register to read from, or None if it is a normal read (non-register read)
    Raises:
        ValueError: if parameters passed into i2c_read are invalid
        Exception: if a non-zero status code was received
    """
    if port !=0 and port != 1:
        raise ValueError("Expected port of 0 (I2C_PORT_1) or 1 (I2C_PORT_2)")
    if address < 0 or address > 255:
        raise ValueError("Expected an uint8 address between 0 and 255")
    if rx_len < 0 or rx_len > 255:
        raise ValueError("Expected an uint8 response length value between 0 and 255")
    if reg < 0 or reg > 255:
        raise ValueError("Expected an uint8 register between 0 and 255")
    
    is_reg = 0
    if reg == None:
        reg = 0
    else:
        is_reg = 1
    # send can message
    data_pack = can_util.can_pack([(port, 1), (address, 1), (rx_len, 1), (is_reg, 1), (reg, 1)])
    can_util.send_message(
        babydriver_id = BabydriverMessageId.I2C_READ_COMMAND,
        data = data_pack
    )

    read_data = []
    num_msgs = ceil(rx_len/7)
    # Loop to receive the ceil(rx_len/7) CAN messages, with only the last 7 bytes of each msg being data
    # and storing only rx_len bytes of data to the read_data list
    for message in range(num_msgs):
        read_msg = can_util.next_message(babydriver_id = BabydriverMessageId.I2C_READ_DATA)
        if message == (num_msgs - 1) and (rx_len % 7) != 0:
            read_data.extend(read_msg.data[1:rx_len % 7 + 1])
        else:
            read_data.extend(read_msg.data[1:8])
    
    # Wait to receive the status CAN message
    status_msg = can_util.next_message(babydriver_id = BabydriverMessageId.STATUS)
    status = status_msg.data[1]

    # Check if status is not STATUS_CODE_OK (0)
    if status != 0:
        raise Exception("Received STATUS_CODE {}".format(status))

    return read_data

