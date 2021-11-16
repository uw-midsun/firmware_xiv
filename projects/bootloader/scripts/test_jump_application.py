# pylint: skip-file
"""This Module Tests the functions in jump_application.py"""

import unittest
import can

from can_datagram import Datagram
from can_datagram import DatagramSender
from can_datagram import DatagramListener

from jump_application import jump_to_application

STATUS_CODE_OK = 0

TEST_CHANNEL = "vcan0"

TEST_DATAGRAM_TYPE_ID = 5
TEST_RESPONSE_DATAGRAM_TYPE_ID = 0
TEST_NODES_IDS = [1, 2, 3, 6, 7, 9]
TEST_DATA = []

TEST_STATUSES = {
    1: STATUS_CODE_OK,
    2: STATUS_CODE_OK,
    3: STATUS_CODE_OK,
    6: STATUS_CODE_OK,
    7: STATUS_CODE_OK,
    9: STATUS_CODE_OK
}


class TestJumpApplication(unittest.TestCase):
    """Test Jump Application"""

    def test_jump_application(self):
        """Test the jump to application feature"""
        self.callback_triggered = False
        self.sender = DatagramSender(channel=TEST_CHANNEL, receive_own_messages=True)
        listener = DatagramListener(self.callback)

        notifier = can.Notifier(self.sender.bus, [listener])

        statuses = jump_to_application(TEST_NODES_IDS, self.sender)

        self.assertEqual(statuses, TEST_STATUSES)
        self.assertEqual(self.callback_triggered, True)

    def callback(self, msg, board_id):
        """Confirms callback is called and datagram_id. Also simulates response datagrams"""
        self.callback_triggered = True
        self.assertEqual(msg.data, TEST_DATAGRAM_TYPE_ID)
        self.mock_responses()

    def mock_responses(self):
        """Sends response datagrams based on node_ids"""
        for node_id in TEST_NODES_IDS:
            mock_message = Datagram(
                datagram_type_id=TEST_RESPONSE_DATAGRAM_TYPE_ID,
                node_ids=node_id,
                data=bytearray(STATUS_CODE_OK))
            self.sender.send(mock_message)


if __name__ == "__main__":
    unittest.main()
