"""This script handles the client side ping protocol"""

# pylint: disable=W0612

# Simple life check. The client sends out a list of IDs, or none to ping all boards
# on the network.
# Each board sends back a message with its ID if it’s in the bootloader
# and ready to receive commands.

import time
import can

from can_datagram import Datagram, DatagramSender, DatagramListener

STATUS_CODE_OK = 0

CLIENT_ID = 0

PING_TYPE_ID = 3
PING_DATA = [] # no data

# For testing purposes
RESPONSE_DATAGRAM_TYPE_ID = 4
RESPONSE_DATAGRAM_NODE_IDS = [CLIENT_ID]
TEST_NODE_IDS = [0,1,2,3] # Some board IDs for testing

def ping(node_ids, sender: DatagramSender):
    """Ping Protocol"""

    # Avoids mutating outside object
    node_ids_copy = set(node_ids)

    recv_boards_statuses = {} # wheter or not 'in bootloader'
    received_all_boards = False

    pinging_all_boards = False
    if node_ids == [0]:
        pinging_all_boards = True

    if not pinging_all_boards:
        # init recv_boards_statuses to flag boards as 'not in bootloader'
        for board_id in node_ids:
            recv_boards_statuses[board_id] = False

    # Nested to have access to above variables
    def trigger_callback(msg, board_id):
        """Listener callback that pairs board_ids with their status"""
        nonlocal received_all_boards
        nonlocal recv_boards_statuses

        # Mocks controller board response datagrams
        # Only runs when testing ('receive_own_messages' is true)
        if msg.datagram_type_id == PING_TYPE_ID:
            if pinging_all_boards:
                # 'All' boards need to respond
                for node_id in TEST_NODE_IDS:
                    mock_message = Datagram(
                        datagram_type_id=RESPONSE_DATAGRAM_TYPE_ID,
                        node_ids=RESPONSE_DATAGRAM_NODE_IDS,
                        data=bytearray([node_id]))
                    sender.send(mock_message, node_id)
            else:
                # only the boards requested need to respond
                for node_id in node_ids:
                    if node_id == 6:
                        # Mock that board 6 is not in bootloader
                        continue
                    mock_message = Datagram(
                        datagram_type_id=RESPONSE_DATAGRAM_TYPE_ID,
                        node_ids=RESPONSE_DATAGRAM_NODE_IDS,
                        data=bytearray([node_id]))
                    sender.send(mock_message, node_id)

        elif msg.datagram_type_id == RESPONSE_DATAGRAM_TYPE_ID:
            # To satisfy while loop condition
            if not pinging_all_boards:
                node_ids_copy.remove(board_id)
                if len(node_ids_copy) == 0:
                    received_all_boards=True
        else:
            raise Exception("Ping Failed - Unknown Datagram ID")

        # Verify data from controller board, and record as 'in bootloader'
        data = int.from_bytes(msg.data, "little")
        if data == board_id:
            recv_boards_statuses[board_id] = True

    listener = DatagramListener(trigger_callback)
    notifier = can.Notifier(sender.bus, [listener])
    message = Datagram(
        datagram_type_id=PING_TYPE_ID,
        node_ids=list(node_ids_copy),
        data=bytearray(PING_DATA))

    sender.send(message)

    timeout = time.time() + 2
    while not received_all_boards:
        if time.time() > timeout:
            break
    return recv_boards_statuses
