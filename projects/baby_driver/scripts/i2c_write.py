"""Python implementation of the i2c_write function"""
import can_util
from message_defs import BabydriverMessageId


def i2c_write(port,address, tx_bytes, reg=None):
    """
    Writes over I2C to the given port/address
    Args:
        port: port of the I2C to write to (1 or 2)
        address: I2C address to write to (0-255)
        tx_bytes: list of bytes to write over I2C (a list of ints 0-255)
        reg: the register to write to, or None if no register is required
    Raises:
        Exception: if a non-zero status code was received
    """
    if port not in (1,0):
        raise ValueError("Expected port of 0 (I2C_PORT_1) or 1 (I2C_PORT_2)")
    if address <0 or address > 255:
        raise ValueError("Expected address between 0 and 255")
    if not all(0 <= byte < 256 for byte in tx_bytes):
        raise ValueError("Expected list of bytes between 0 and 255")

    is_reg = 0
    if reg is not None:
        is_reg = 1
    else:
        reg = 0
    if reg <0 or reg > 255:
        raise ValueError("Expected register to write to between 0 and 255")

    can_pack = can_util.can_pack([(port,1), (address,1), (len(tx_bytes),1), (is_reg,1), (reg,1)])
    can_util.send_message(
        babydriver_id=BabydriverMessageId.I2C_WRITE_COMMAND,
        data=can_pack
    )

    # Sends CAN message
    for i in range(0, len(tx_bytes), 7):
        can_util.send_message(
            babydriver_id=BabydriverMessageId.I2C_WRITE_DATA,
            data=tx_bytes[i:i+8]
            )
    status_msg = can_util.next_message(babydriver_id=BabydriverMessageId.STATUS)
    # Raises Exception if status is non-OK
    if status_msg.data[1] != 0:
        raise Exception("Received STATUS_CODE {}".format(status_msg.data[1]))
