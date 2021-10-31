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

    # 'data' attribute represents whether callback was called or not
    message = Datagram(
        datagram_type_id=5,
        node_ids=node_ids,
        data=False)
    sender.send(message)

    timeout = time.time() + 10
    while not message.data:
        if time.time() > timeout:
            break


def trigger_callback(datagram):
    '''Returns datagram status from boards'''
    # callback has been called, breaking while loop
    datagram.data = True
    print("Response status code is {}".format(datagram.data))
    return datagram.data
