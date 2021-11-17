# pylint: skip-file
"""This client script handles the jump-to-application process of controller boards."""

import time
import can

from can_datagram import Datagram
from can_datagram import DatagramSender
from can_datagram import DatagramListener

DATAGRAM_TYPE_ID = 5
DATAGRAM_DATA = []


def jump_to_application(node_ids, sender: DatagramSender):
    """This function sends a jump-to-application datagram to specified boards,
    receives a response, then returns the status"""
    statuses = {}

    def trigger_callback(msg, board_id):
        """This function is nested because it needs access to the dictionary of board ids to store their corresponding status. It removes the board_id from node_ids to statisfy while loop condition"""
        statuses[board_id] = int.from_bytes(msg.data, "big")
        node_ids.remove(board_id)

    listener = DatagramListener(trigger_callback)

    notifier = can.Notifier(sender.bus, [listener])

    message = Datagram(
        datagram_type_id=DATAGRAM_TYPE_ID,
        node_ids=node_ids,
        data=bytearray(DATAGRAM_DATA))
    sender.send(message)

    timeout = time.time() + 10
    # Retrieves response datagrams until there are none or timeout
    while node_ids != []:
        if time.time() > timeout:
            break
    return statuses
