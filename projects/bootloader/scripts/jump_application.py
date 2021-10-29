"""This client script handles datagram protocol communication between devices on the CAN."""

import can_datagram


def jump_to_application(node_ids):
    '''Sends datagram to specific boards, receives response, then returns status'''
    # creates datagram to be sent to boards
    client_datagram = can_datagram.Datagram(datagram_type_id=5, node_ids=node_ids, data="")
    # sends datagram
    can_datagram.DatagramSender().send(client_datagram)
    # response
    can_datagram.DatagramListener(return_status)


def return_status(datagram):
    '''Returns datagram status from boards'''
    return datagram.data
