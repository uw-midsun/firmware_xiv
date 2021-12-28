"""This script handles the client side jump-to-application protocol"""

# pylint: disable=W0612

import time
import can

from can_datagram import Datagram, DatagramSender, DatagramListener

STATUS_CODE_OK = 0
CLIENT_ID = 0

JUMP_TO_APP_DATAGRAM_TYPE_ID = 5
JUMP_TO_APP_DATAGRAM_DATA = []

# For testing purposes
RESPONSE_DATAGRAM_TYPE_ID = 0
RESPONSE_DATAGRAM_NODE_IDS = [CLIENT_ID]
RESPONSE_DATAGRAM_DATA = [STATUS_CODE_OK]


def jump_to_application(node_ids, sender: DatagramSender):
    """Jump To Application Protocol"""

    recv_boards_statuses = {}
    # Avoids mutating outside object
    node_ids_copy = set(node_ids)
    # Nested to have access to 'recv_board_statuses'

    def trigger_callback(msg, board_id):
        """Listener callback that pairs board_ids with their status"""

        # Mocks controller board response datagrams
        # Only runs when testing ('receive_own_messages' is true)
        if msg.datagram_type_id == JUMP_TO_APP_DATAGRAM_TYPE_ID:
            for node_id in node_ids:
                mock_message = Datagram(
                    datagram_type_id=RESPONSE_DATAGRAM_TYPE_ID,
                    node_ids=RESPONSE_DATAGRAM_NODE_IDS,
                    data=bytearray(RESPONSE_DATAGRAM_DATA))
                sender.send(mock_message, node_id)
        elif msg.datagram_type_id == RESPONSE_DATAGRAM_TYPE_ID:
            # To satisfy while loop condition
            node_ids_copy.remove(board_id)
        else:
            raise Exception("Jump Application Failed - Unknown Datagram ID")
        recv_boards_statuses[board_id] = int.from_bytes(msg.data, "little")

    listener = DatagramListener(trigger_callback)
    notifier = can.Notifier(sender.bus, [listener])
    message = Datagram(
        datagram_type_id=JUMP_TO_APP_DATAGRAM_TYPE_ID,
        node_ids=list(node_ids_copy),
        data=bytearray(JUMP_TO_APP_DATAGRAM_DATA))

    sender.send(message)

    timeout = time.time() + 10
    while node_ids_copy:
        if time.time() > timeout:
            break
    return recv_boards_statuses
