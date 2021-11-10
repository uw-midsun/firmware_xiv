"""This client script handles the jump-to-application process of controller boards."""

import time
import can

from can_datagram import Datagram
from can_datagram import DatagramSender
from can_datagram import DatagramListener

DATAGRAM_TYPE_ID = 5
DATAGRAM_DATA = []


def jump_to_application(node_ids):
    """This function sends jump-to-application datagrams to specified boards,
    receives, response, then returns status"""
    sender = DatagramSender(receive_own_messages=True)
    listener = DatagramListener(trigger_callback)

    can.Notifier(sender.bus, [listener])

    message = Datagram(
        datagram_type_id=DATAGRAM_TYPE_ID,
        node_ids=node_ids,
        data=DATAGRAM_DATA)
    sender.send(message)

    timeout = time.time() + 10
    listener_message = listener.get_message()
    # Retrieves response datagrams until there are none or timeout
    while listener_message is not None:
        if time.time() > timeout:
            break
        listener_message = listener.get_message()


def trigger_callback(msg):
    """Returns status of response datagrams from specified boards"""
    print(f"Response status code of datagram id {msg.datagram_type_id} is {msg.data}")
    return msg.data
