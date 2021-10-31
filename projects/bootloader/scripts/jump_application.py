"""This client script handles the jump-to-application process of controller boards."""

import time
import can_datagram as cd


def jump_to_application(node_ids):
    '''Sends datagram to specific boards, receives response, then returns status'''
    # creates datagram to be sent to boards
    datagram = cd.Datagram(datagram_type_id=5, node_ids=node_ids, data=None)
    # listens for messages and adds them to queue
    listener = cd.DatagramListener(return_status)
    # sends instructions to relevant node ids
    cd.DatagramSender().send(datagram)
    # Source: https://github.com/hardbyte/python-can/issues/352
    time.sleep(5) 
    msg = None
    while msg is None:
        # retrieves messages in queue
        msg = listener.get_message()
        if msg:
            listener.on_message_received(msg)


def return_status(datagram):
    '''Returns datagram status from boards'''
    print("Response status code is {}".format(datagram.data))
    return datagram.data
