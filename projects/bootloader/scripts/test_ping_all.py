"""This Module Tests functions in ping.py"""

import unittest

from can_datagram import DatagramSender
from ping import ping

STATUS_CODE_OK = 0

TEST_CHANNEL = "vcan0"

TEST_NODE_IDS = [0]

TEST_EXPECTED_STATUSES = {
    0: True,
    1: True,
    2: True,
    3: True,
}

class TestPing(unittest.TestCase):
    """Test Ping functions"""

    def test_ping_all(self):
        """Tests the ping function by pinging all ids implicitly"""

        sender = DatagramSender(channel=TEST_CHANNEL, receive_own_messages=True)
        recv_boards_statuses = ping(TEST_NODE_IDS, sender)
        self.assertEqual(recv_boards_statuses, TEST_EXPECTED_STATUSES)

if __name__ == "__main__":
    unittest.main()
