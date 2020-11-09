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
