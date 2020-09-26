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


def get_arbitration_id(msg_id, device_id):
    """Returns the arbitration ID for a CAN message from its message and device ID."""
    # our CAN system uses 6 bits of message ID, 1 bit for ACK/DATA, and 4 bits for device ID
    return (msg_id << 5) | device_id


def arbitration_to_message_id(arbitration_id):
    """Extracts and returns the message ID from a CAN message's arbitration ID."""
    # message ID is bits 5-11
    return (arbitration_id >> 5) & 0b111111


def send_message(
        babydriver_id,
        data,
        channel=None,
        msg_id=BABYDRIVER_CAN_MESSAGE_ID,
        device_id=BABYDRIVER_DEVICE_ID,
):
    """Sends a CAN message.

    Args:
        babydriver_id: The babydriver ID (first byte of message data) of the message to send.
        data: Up to 7 bytes (0-255) of data to send in the CAN message.
        channel: The SocketCAN channel on which to send the message.
        msg_id: The CAN message ID to use.
        device_id: The device ID to use.

    Raises:
        can.CanError: If there was an error in transmitting the message.
    """

    if len(data) > 7:
        raise ValueError("Only 7 bytes of data may be sent")
    if len(data) < 7:
        # pad to 7 bytes so that the firmware project will accept it
        data += [0] * (7 - len(data))

    message_data = bytearray([babydriver_id] + data)

    bus = get_bus(channel)

    msg = can.Message(
        arbitration_id=get_arbitration_id(msg_id, device_id),
        data=message_data,
        is_extended_id=False
    )

    bus.send(msg)


def next_message(
        babydriver_id=None,
        channel=None,
        timeout=1,
        msg_id=BABYDRIVER_CAN_MESSAGE_ID,
):
    """Blocks until we receive a babydriver CAN message or we time out.

    Args:
        babydriver_id: If non-None and the received message's babydriver ID (i.e. first byte of
            message data) isn't equal to this, raise an exception.
        channel: The SocketCAN channel to send on (can0 or vcan0).
        timeout: Timeout to wait for a message before raising an exception, in seconds.
        msg_id: The CAN message ID to wait for, defaulting to the babydriver CAN message. All other
            CAN messages will be ignored.

    Returns:
        A bytearray of data from the CAN message received.

    Raises:
        TimeoutError: if we time out waiting for an appropriate CAN message.
        ValueError: if we receive a CAN message but its babydriver ID does not match.
    """

    bus = get_bus(channel)

    time_left = timeout
    current_time = time.time()
    msg = None

    while time_left > 0:
        msg = bus.recv(timeout=time_left)
        if msg is None:
            # bus.recv timed out
            break

        rx_msg_id = arbitration_to_message_id(msg.arbitration_id)
        if rx_msg_id == msg_id:
            break

        # ignore messages that we aren't waiting for.
        msg = None

        new_time = time.time()
        time_left -= new_time - current_time
        current_time = new_time

    if msg is None:
        raise TimeoutError()

    if babydriver_id is not None and (not msg.data or msg.data[0] != babydriver_id):
        raise ValueError("next_message expected babydriver ID {} but got {}".format(
            babydriver_id,
            msg.data[0] if msg.data else "empty message",
        ))

    return msg.data
