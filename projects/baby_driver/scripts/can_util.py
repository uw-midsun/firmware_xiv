"""This module implements a low-level wrapper over Python-CAN's CAN API specific to Babydriver."""

import time
import can

from message_defs import BABYDRIVER_DEVICE_ID, BABYDRIVER_CAN_MESSAGE_ID


# The default CAN channel to use for this module. Changed dynamically by cli_setup.
default_channel = "can0" # pylint: disable=invalid-name


def get_bus(channel=None):
    """Returns a new Python-CAN Bus for sending/receiving messages."""

    if channel is None:
        channel = default_channel

    return can.interface.Bus(bustype="socketcan", channel=channel, bitrate=500000)


class Message:
    """
    An immutable wrapper over Python-CAN's can.Message to support our message and device ID
    conventions. See https://python-can.readthedocs.io/en/master/message.html.

    Attributes:
        msg: The underlying can.Message.
        message_id: The message ID of the CAN message.
        device_id: The device ID of the CAN message.
        data: The data associated with the message.
    """

    def __init__(self, message_id=0, device_id=0, **kwargs):
        """Initialize a Message. See Python-CAN's can.Message for more info.

        Args:
            message_id: The message ID of the CAN message, used if arbitration_id is not passed.
            device_id: The device ID of the CAN message, used if arbitration_id is not passed.
        """

        if "arbitration_id" not in kwargs:
            # our CAN system uses 6 bits of message ID, 1 bit for ACK/DATA, and 4 bits for device ID
            kwargs["arbitration_id"] = (message_id << 5) | device_id

        self.msg = can.Message(**kwargs)

    @classmethod
    def from_msg(cls, msg):
        """Helper to get a Message from a can.Message."""
        message = cls()
        message.msg = msg
        return message

    @property
    def message_id(self):
        """The message ID of this CAN message."""
        # message ID is bits 5-11
        return (self.msg.arbitration_id >> 5) & 0b111111

    @property
    def device_id(self):
        """The device ID of this CAN message."""
        # device ID is bits 0-3
        return self.msg.arbitration_id & 0b1111

    @property
    def data(self):
        """The data associated with this CAN message."""
        return self.msg.data


def send_message(
        babydriver_id=None,
        data=None,
        channel=None,
        msg_id=BABYDRIVER_CAN_MESSAGE_ID,
        device_id=BABYDRIVER_DEVICE_ID,
):
    """Sends a CAN message.

    Args:
        babydriver_id: The babydriver ID (first byte of message data) of the message to send. If
            None, the first byte of message data isn't overwritten.
        data: The data to send in the CAN message. Must be a list of bytes (0-255). If babydriver_id
            is None, this can be up to 8 bytes; otherwise, it can only be up to 7 bytes since the
            first byte is the babydriver ID.
        channel: The SocketCAN channel on which to send the message.
        msg_id: The CAN message ID to use.
        device_id: The device ID to use.

    Raises:
        can.CanError: If there was an error in transmitting the message.
    """

    if data is None:
        data = []
    if babydriver_id is not None:
        data = [babydriver_id] + data

    if len(data) > 8:
        raise ValueError("Only 8 bytes of data (including babydriver ID) may be sent")
    if len(data) < 8 and msg_id == BABYDRIVER_CAN_MESSAGE_ID:
        # pad to 8 bytes so that the firmware project will accept it
        data += [0] * (8 - len(data))

    data = bytearray(data)

    bus = get_bus(channel)

    msg = Message(
        message_id=msg_id,
        device_id=device_id,
        data=data,
        is_extended_id=False
    )

    bus.send(msg.msg)


def next_message(
        babydriver_id=None,
        channel=None,
        timeout=1,
        msg_id=BABYDRIVER_CAN_MESSAGE_ID,
):
    """Blocks until we receive a babydriver CAN message or we time out.

    Args:
        babydriver_id: A babydriver ID or list of IDs. If non-None and the received message's
            babydriver ID (i.e. first byte of message data) isn't equal to this or an element of
            this, raise an exception.
        channel: The SocketCAN channel to send on (can0 or vcan0).
        timeout: Timeout to wait for a message before raising an exception, in seconds.
        msg_id: The CAN message ID or list of IDs to wait for, defaulting to the babydriver CAN
            message. All other CAN messages will be ignored. If None, don't check the message ID
            and return the first CAN message we see.

    Returns:
        A Message object representing the received CAN message.

    Raises:
        TimeoutError: if we time out waiting for an appropriate CAN message.
        ValueError: if we receive a CAN message but its babydriver ID does not match.
    """

    # make these iterable to support waiting on one or multiple message/babydriver IDs
    if isinstance(babydriver_id, int):
        babydriver_id = (babydriver_id,)
    if isinstance(msg_id, int):
        msg_id = (msg_id,)

    bus = get_bus(channel)

    time_left = timeout
    current_time = time.time()
    msg = None

    while time_left > 0:
        msg = bus.recv(timeout=time_left)
        if msg is None:
            # bus.recv timed out
            break

        msg = Message.from_msg(msg)
        if msg_id is None or msg.message_id in msg_id:
            break

        # ignore messages that we aren't waiting for.
        msg = None

        new_time = time.time()
        time_left -= new_time - current_time
        current_time = new_time

    if msg is None:
        raise TimeoutError()

    if babydriver_id is not None and (not msg.data or msg.data[0] not in babydriver_id):
        raise ValueError("next_message expected babydriver ID {} but got {}".format(
            babydriver_id,
            msg.data[0] if msg.data else "empty message",
        ))

    return msg

def can_pack(data_list):
    """
    Converts list of tuples in form ((int) val, (int) len_in_bytes)
            and combines them into a bytearray rendition
    Returns:
        A bytearray object representing message components passed in
    Raises:
        ValueError: if tuple has more than 2 items
        ValueError: if tuple value exceeds allotted length in bytes
    """
    to_bytes = ""
    for item in data_list:
        if len(item) > 2:
            raise ValueError("Tuple length {}: expected 2".format(len(item)))
        #store value and number of bytes allotted
        val = item[0]
        len_in_bytes = item[1]
        #check that values are valid
        if len_in_bytes < 1 or val < 0:
            raise ValueError("len in bytes must be > 0; val must be non-negative")
        if val >= pow(2, len_in_bytes * 8):
            raise ValueError("Value {} exceeds allotted {} bytes. Max Val: {}".format(
                val, len_in_bytes, pow(2, len_in_bytes * 8) - 1))
        hex_len = 2 * len_in_bytes
        hex_str = '{0:x}'.format(val)
        #prepend zeros to match number of bytes allotted
        while len(hex_str) < hex_len:
            hex_str = '0' + hex_str
        to_bytes += hex_str
    return bytearray.fromhex(to_bytes)
    