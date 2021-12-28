"""This Module Tests functions in jump_application.py"""

import unittest

from can_datagram import DatagramSender
from jump_application import jump_to_application

STATUS_CODE_OK = 0

TEST_CHANNEL = "vcan0"

TEST_NODE_IDS = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]

TEST_EXPECTED_STATUSES = {
    0: STATUS_CODE_OK,
    1: STATUS_CODE_OK,
    2: STATUS_CODE_OK,
    3: STATUS_CODE_OK,
    4: STATUS_CODE_OK,
    5: STATUS_CODE_OK,
    6: STATUS_CODE_OK,
    7: STATUS_CODE_OK,
    8: STATUS_CODE_OK,
    9: STATUS_CODE_OK,
    10: STATUS_CODE_OK,
}


class TestJumpApplication(unittest.TestCase):
    """Test Jump Application Code functions"""

    def test_jump_application(self):
        """Tests the jump_to_application function"""
        sender = DatagramSender(channel=TEST_CHANNEL, receive_own_messages=True)
        recv_boards_statuses = jump_to_application(TEST_NODE_IDS, sender)
        self.assertEqual(recv_boards_statuses, TEST_EXPECTED_STATUSES)


if __name__ == "__main__":
    unittest.main()
