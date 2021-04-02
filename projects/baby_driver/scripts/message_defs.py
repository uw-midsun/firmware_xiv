"""Constants for the babydriver CAN protocol."""
# pylint: disable=too-few-public-methods

# Both the C and Python babydriver projects use 15 as the device ID, see can_msg_defs.h.
BABYDRIVER_DEVICE_ID = 15

# The babydriver CAN message has ID 63, see can_msg_defs.h.
BABYDRIVER_CAN_MESSAGE_ID = 63


class BabydriverMessageId:
    """
    An enumeration of babydriver IDs, which go in the first uint8 in a babydriver CAN message.

    This is the Python equivalent of the enum of the same name in babydriver_msg_defs.h and should
    be kept up to date with it.
    """

    STATUS = 0
    GPIO_SET = 1
    GPIO_GET_COMMAND = 2
    GPIO_GET_DATA = 3
    ADC_READ_COMMAND = 4
    ADC_READ_DATA = 5
    I2C_READ_COMMAND = 6
    I2C_READ_DATA = 7
    I2C_WRITE_COMMAND = 8
    I2C_WRITE_DATA = 9
    SPI_EXCHANGE_METADATA_1 = 10
    SPI_EXCHANGE_METADATA_2 = 11
    SPI_EXCHANGE_TX_DATA = 12
    SPI_EXCHANGE_RX_DATA = 13
    GPIO_IT_REGISTER_COMMAND = 14
    GPIO_IT_UNREGISTER_COMMAND = 15
    GPIO_IT_INTERRUPT = 16
