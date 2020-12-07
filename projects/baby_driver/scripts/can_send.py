"""This module sends user-specified CAN messages, without needing to first send them through the STM32."""

import cantools
import can_util
from message_defs import BABYDRIVER_DEVICE_ID

# global var for the Database from load_dbc
DBC_DATABASE = None

def can_send_raw(msg_id, data, device_id=BABYDRIVER_DEVICE_ID, channel=None):
    """
    A wrapper over can_utils.send_message.
    Args:
        msg_id: CAN message ID
            (if it's for babydriver messages, then msg_id=BABYDRIVER_CAN_MESSAGE_ID)
        data: a list of up to 8 bytes of data to send
        device_id: the device ID to send from
        channel: CAN channel
    """
    try:
        can_util.send_message(
            data=data,
            channel=channel,
            msg_id=msg_id,
            device_id=device_id,
        )
    except: # pylint: disable=raise-missing-from
        raise Exception("Raw CAN message could not be sent.")

def load_dbc(dbc_filename):
    """
    Creates a Database object from an existing DBC file used to encode CAN messages.
    Args:
        dbc_filename: a string representing the path to a DBC file
    """

    DBC_DATABASE = cantools.database.load_file(dbc_filename)

    if not DBC_DATABASE:
        raise Exception("DBC file was not loaded.")

def can_send(msg_name, channel=None, **data):
    """
    Uses the Database object created by the load_dbc function to encode a CAN message
    Args:
        msg_name: the string name of the message
        channel: CAN channel
        data: a dictionary of field names (strings) to values (integers)
    """
    try:
        msg_obj = DBC_DATABASE.get_message_by_name(msg_name.upper())
        msg_obj = msg_obj.encode(data)
    except: # pylint: disable=raise-missing-from
        raise Exception("CAN message could not be encoded with dbc data.")

    bus = can_util.get_bus(channel)
    can_msg = can_util.Message(arbitration_id=msg_obj.frame_id, data=msg_obj)
    bus.send(msg_obj.msg)
