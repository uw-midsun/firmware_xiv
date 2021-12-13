"""This Module Tests methods in flash_application_code.py"""

import unittest
from projects.bootloader.scripts.flash_application_code import FlashApplication

TEST_PROTOCOL_VERSION = 1
TEST_DATAGRAM_TYPE_ID = 1
# Include client ID for testing
TEST_NODES = [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
TEST_DATA = [3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5, 8, 9, 7, 9, 3, 2, 3, 8, 4, 6, 2, 6, 4, 3, 3]
TEST_CHANNEL = "vcan0"
TEST_NAME = "TestName"
TEST_GIT_VERSION = 1


class TestFAC(unittest.TestCase):
    """Test Flash Application Code functions"""

    def test_flash_application(self):
        """Test function following the same process as flash_application_code"""

        flash = FlashApplication(
            channel=TEST_CHANNEL,
            board_ids=TEST_NODES,
            application_code_data=bytes(TEST_DATA),
            name=TEST_NAME,
            git_version=TEST_GIT_VERSION,
            sender_receive_own_messages=True
        )

        flash.flash_protobuf()

        expected_nodes = set(TEST_NODES)
        self.assertEqual(flash.status_code, 0)
        self.assertEqual(flash.recv_boards, expected_nodes)
        self.assertEqual(flash.status_code, 0)

        chunked_application_code = flash.chunks(flash.app_data, 2048)
        for chunk in chunked_application_code:
            # Reset recieved keys in listener
            flash.listener.datagram_messages = {}
            flash.recv_boards = set()

            flash.flash_application_chunk(chunk)

        self.assertEqual(flash.recv_boards, expected_nodes)
        self.assertEqual(flash.status_code, 0)


if __name__ == '__main__':
    unittest.main()
