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
    handles responses, then returns the statuses"""
    statuses = {}

    # Avoids mutating outside object
    node_ids_copy = node_ids.copy()

    def trigger_callback(msg, board_id):
        """This function is nested because it requires access to the statuses dictionary"""
        # Paases when datagram isn't a jump-to-application one
        if msg.datagram_type_id != DATAGRAM_TYPE_ID:
            statuses[board_id] = int.from_bytes(msg.data, "big")
            # Deals with while loop condition
            node_ids_copy.remove(board_id)

    listener = DatagramListener(trigger_callback)

    notifier = can.Notifier(sender.bus, [listener])

    message = Datagram(
        datagram_type_id=DATAGRAM_TYPE_ID,
        node_ids=node_ids_copy,
        data=bytearray(DATAGRAM_DATA))
    sender.send(message)

    timeout = time.time() + 10
    # Retrieves response datagrams until there are none or timeout
    while node_ids_copy != []:
        if time.time() > timeout:
            break
    return statuses
