# pylint: skip-file
"""This Module Tests functions in jump_application.py"""

import unittest
import can

from can_datagram import Datagram
from can_datagram import DatagramSender
from can_datagram import DatagramListener

from jump_application import jump_to_application

STATUS_CODE_OK = 0

TEST_CHANNEL = "vcan0"

TEST_DATAGRAM_TYPE_ID = 5
TEST_NODE_IDS = [1, 2, 3, 4]
TEST_DATA = []

TEST_RESPONSE_DATAGRAM_TYPE_ID = 0
TEST_RESPONSE_NODE_IDS = [TEST_RESPONSE_DATAGRAM_TYPE_ID]
TEST_RESPONSE_DATA = [STATUS_CODE_OK]

TEST_STATUSES = {
    1: STATUS_CODE_OK,
    2: STATUS_CODE_OK,
    3: STATUS_CODE_OK,
    4: STATUS_CODE_OK,
}


class TestJumpApplication(unittest.TestCase):
    """Test Jump Application"""

    def test_jump_application(self):
        """Test the jump to application feature"""
        self.callback_triggered = False
        self.sender = DatagramSender(channel=TEST_CHANNEL, receive_own_messages=True)
        listener = DatagramListener(self.callback)

        notifier = can.Notifier(self.sender.bus, [listener])

        statuses = jump_to_application(TEST_NODE_IDS, self.sender)

        self.assertEqual(self.callback_triggered, True)
        self.assertEqual(statuses, TEST_STATUSES)

    def callback(self, msg, board_id):
        """This callback is called when the initial jump-to-application datagram is sent"""
        self.callback_triggered = True
        if board_id == TEST_RESPONSE_DATAGRAM_TYPE_ID:
            self.assertEqual(msg.datagram_type_id, TEST_DATAGRAM_TYPE_ID)
            self.mock_responses()

    def mock_responses(self):
        """This function simulates sending back response datagrams"""
        for node_id in TEST_NODE_IDS:
            print("response datagram being sent...")
            mock_message = Datagram(
                datagram_type_id=TEST_RESPONSE_DATAGRAM_TYPE_ID,
                node_ids=TEST_RESPONSE_NODE_IDS,
                data=bytearray(TEST_RESPONSE_DATA))
            self.sender.send(mock_message, node_id)


if __name__ == "__main__":
    unittest.main()
