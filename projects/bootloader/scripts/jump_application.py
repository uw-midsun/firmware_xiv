"""This client script handles the jump-to-application process of controller boards."""

import time

from can_datagram import Datagram
from can_datagram import DatagramSender
from can_datagram import DatagramListener


def jump_to_application(node_ids):
    '''Sends datagram to specific boards, receives response, then returns status'''
    sender = DatagramSender(receive_own_messages=True)

    listener = DatagramListener(trigger_callback)
    notifier = can.Notifier(sender.bus, [listener])

    message = Datagram(
        datagram_type_id=5,
        node_ids=node_ids,
        data=[])
    sender.send(message)

    listener_message = listener.get_message()
    while listener_message is not None:
        listener.on_message_received(listener_message)
        listener_message = listener.get_message()


def trigger_callback(datagram):
    '''Returns datagram status from boards'''
    print("Response status code is {}".format(datagram.data))
    return datagram.data
