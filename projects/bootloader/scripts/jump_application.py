"""This client script handles the jump-to-application process of controller boards."""

import can_datagram


def jump_to_application(node_ids):
    '''Sends datagram to specific boards, receives response, then returns status'''
    # creates datagram to be sent to boards
    client_datagram = can_datagram.Datagram(datagram_type_id=5, node_ids=node_ids, data="")
    # sends datagram
    # do this check for each node id,
    can_datagram.DatagramSender().send(client_datagram)
    # response
    datagram_listener = can_datagram.DatagramListener(return_status)
    while True:
        # if there is a message received:
        datagram_listener.on_message_received("")


def return_status(datagram):
    #
    '''Returns datagram status from boards'''
    return datagram.data
