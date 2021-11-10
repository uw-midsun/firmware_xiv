"""This Module Tests the functions in jump_application.py"""

import unittest
import time
import can

from can_datagram import Datagram
from can_datagram import DatagramSender
from can_datagram import DatagramListener

TEST_CHANNEL = "vcan0"

TEST_DATAGRAM_TYPE_ID = 5
TEST_NODES = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
TEST_DATA = []

TEST_RESPONSE_DATAGRAM_TYPE_ID = 0

STATUS_CODE_OK = 0


class TestJumpApplication(unittest.TestCase):
    """Test Jump Application"""

    def test_jump_application(self):
        """Test the jump to application feature"""
        sender = DatagramSender(channel=TEST_CHANNEL, receive_own_messages=True)
        listener = DatagramListener(self.callback)

        can.Notifier(sender.bus, [listener])

        message = Datagram(
            datagram_type_id=TEST_DATAGRAM_TYPE_ID,
            node_ids=TEST_NODES,
            data=bytearray(TEST_DATA))
        sender.send(message)

        timeout = time.time() + 10
        listener_message = listener.get_message()
        while listener_message is not None:
            if time.time() > timeout:
                break
            listener_message = listener.get_message()

    def callback(self, msg):
        """Received datagram should have correct status and id"""
        if msg.datagram_type_id == TEST_DATAGRAM_TYPE_ID:
            return
        self.assertEqual(msg.datagram_type_id, TEST_RESPONSE_DATAGRAM_TYPE_ID)
        self.assertEqual(msg.data, bytearray(STATUS_CODE_OK))


if __name__ == "__main__":
    unittest.main()
