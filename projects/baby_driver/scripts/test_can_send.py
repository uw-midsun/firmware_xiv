"""This Module Tests methods in can_send.py"""
# pylint: disable=unused-import

import unittest
from unittest.mock import patch

import can
import can_util
import cantools
from can_send import can_send_raw, load_dbc, can_send


class TestCanSendRaw(unittest.TestCase):
    """Tests functions in Babydriver's can_send module"""

    @patch('can_util.send_message')
    def can_send_raw(self, mock_send_message):
        """Tests that can_send_raw raises an Exception if CAN msg fails to send"""
        mock_send_message.return_value = can.CanError
        mock_msg_id = 0
        mock_data = ["item1", "item2", "item3"]
        self.assertRaises(Exception, can_send_raw, mock_msg_id, mock_data)

    @patch('cantools.database.load_file')
    def test_load_dbc(self, mock_cantools_load_file):
        """Tests that load_dbc raises an Exception if no file is found"""
        mock_cantools_load_file.return_value = None
        self.assertRaises(Exception, load_dbc, "./some-file-path")

    @patch('cantools.database.can.Message.encode')
    def test_can_send(self, mock_encode):
        """Tests that can_send raises an Exception if no msg_obj is created"""
        mock_encode.return_value = None
        self.assertRaises(Exception, can_send, "ex-msg-name")

if __name__ == '__main__':
    unittest.main()
