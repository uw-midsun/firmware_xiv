"""This module implements a low-level wrapper over Python-CAN's CAN API specific to Babydriver."""

import time
from collections import namedtuple

import can

from message_defs import BABYDRIVER_DEVICE_ID, BABYDRIVER_CAN_MESSAGE_ID


# The default CAN channel to use for this module. Changed dynamically by repl_setup.
default_channel = "can0"  # pylint: disable=invalid-name

# We use the standard 500kbps baudrate.
CAN_BITRATE = 500000

# A mapping for caching data about each channel, and a handy type used for the cached data.
# We use a cache so that there's only one notifier thread running per channel.
channels_to_data = {}
ChannelData = namedtuple("ChannelData", ["bus", "reader", "notifier"])


def get_bus_data(channel=None):
    """Returns the memoized ChannelData for a given channel"""

    if channel is None:
        channel = default_channel

    if channel not in channels_to_data:
        bus = can.interface.Bus(bustype="socketcan", channel=channel, bitrate=CAN_BITRATE)

        # We use a BufferedReader rather than reading straight from the Bus to prevent the scenario
        # where the time for the second message to be tx'd exceeds the time taken to process the
        # first message and start listening for the second one, which occurs if the C side is fast
        # enough. By buffering, we never miss a message.
        reader = can.BufferedReader()
        notifier = can.Notifier(bus, [reader])

        channels_to_data[channel] = ChannelData(bus, reader, notifier)

    return channels_to_data[channel]


def get_bus(channel=None):
    """Returns a Python-CAN Bus for sending messages."""
    return get_bus_data(channel).bus


def get_bus_reader(channel=None):
    """Returns a Python-CAN BufferedReader for reading from the specified channel."""
    return get_bus_data(channel).reader


def get_bus_notifier(channel=None):
    """Returns a Python-CAN Notifier to listen for can messages."""
    return get_bus_data(channel).notifier


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

    reader = get_bus_reader(channel)

    time_left = timeout
    current_time = time.time()
    msg = None

    while time_left > 0:
        msg = reader.get_message(timeout=time_left)
        if msg is None:
            # reader.get_message timed out
            break

        # ignore anything sent before 100ms ago to avoid bad messages causing future errors
        if msg.timestamp < current_time - 0.1:
            continue

        msg = Message.from_msg(msg)
        if msg_id is None or msg.message_id in msg_id:
            break

        # ignore messages that we aren't waiting for.
        msg = None

        new_time = time.time()
        time_left -= new_time - current_time
        current_time = new_time

    if msg is None:
        raise TimeoutError

    if babydriver_id is not None and (not msg.data or msg.data[0] not in babydriver_id):
        raise ValueError("next_message expected babydriver ID {} but got {}".format(
            babydriver_id,
            msg.data[0] if msg.data else "empty message",
        ))

    return msg


def can_pack(data_list):
    """
    Converts list of tuples and combines them into an array
    rendition. Each val is broken into individual byte values
    and appended to bytearr output (LSB first)
    Args:
        List of tuples in form ((int) val, (int) len_in_bytes). val must be
        in range [0, 2**len_in_bytes - 1]
    Returns:
        An array of byte values in little endian format, representing
        message components input
    Raises:
        ValueError: if invalid values for val, len_in_bytes input
    """
    bytearr = []
    # Traverse list
    for val, len_in_bytes in data_list:
        # Error check input vals
        if len_in_bytes < 1 or val < 0:
            raise ValueError("len in bytes must be > 0; val must be non-negative")
        if val >= 1 << (len_in_bytes * 8):
            raise ValueError("Value {} exceeds allotted {} bytes. Max Val: {}".format(
                val, len_in_bytes, 1 << (len_in_bytes * 8) - 1))
        # Split val into bytes rendition, re-pack in little-endian
        for _ in range(len_in_bytes):
            int_out = val & 0xFF
            bytearr.append(int_out)
            val = val >> 8
    return bytearr
