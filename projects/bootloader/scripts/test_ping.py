"""This Module Tests functions in ping.py"""

import unittest

from can_datagram import DatagramSender
from ping import ping

STATUS_CODE_OK = 0

TEST_CHANNEL = "vcan0"

TEST_NODE_IDS = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]

TEST_EXPECTED_STATUSES = {
    0: True,
    1: True,
    2: True,
    3: True,
    4: True,
    5: True,
    6: False, # Mock controller board 6 as 'not in bootloader'
    7: True,
    8: True,
    9: True,
    10:True,
}

class TestPing(unittest.TestCase):
    """Test Ping functions"""

    def test_ping_specific(self):
        """Tests the ping function by pinging specific ids"""

        sender = DatagramSender(channel=TEST_CHANNEL, receive_own_messages=True)
        recv_boards_statuses = ping(TEST_NODE_IDS, sender)
        self.assertEqual(recv_boards_statuses, TEST_EXPECTED_STATUSES)

if __name__ == "__main__":
    unittest.main()
