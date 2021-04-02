"""This Module Tests methods in can_send.py"""
# pylint: disable=unused-import

import unittest
from unittest.mock import patch, Mock

import can
import cantools
import can_util
from message_defs import BABYDRIVER_DEVICE_ID
from can_send import can_send_raw, load_dbc, can_send


class TestCanSendRaw(unittest.TestCase):
    """Tests functions in Babydriver's can_send module"""

    @patch('can_util.send_message')
    def test_can_send_raw_parameters(self, mock_send_message):
        """Tests accuracy of parameters passed into can_send_raw"""

        # Stores parameters passed into can_util.send_message
        # pylint: disable=attribute-defined-outside-init
        self.msg_id = None
        self.data = None
        self.device_id = None
        self.channel = None

        # pylint: disable=missing-docstring
        def parameter_test(msg_id, data, device_id=BABYDRIVER_DEVICE_ID, channel=None):
            self.msg_id = msg_id
            self.data = data
            self.device_id = device_id
            self.channel = channel

        # Checks whether parameters passed into can_send_raw match
        # parameters passed into parameter_test
        mock_send_message.side_effect = parameter_test
        can_send_raw(0, [10, 255, 0], BABYDRIVER_DEVICE_ID, None)
        self.assertEqual(0, self.msg_id)
        self.assertEqual([10, 255, 0], self.data)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)
        self.assertEqual(None, self.channel)

    @patch('can_util.send_message')
    def test_can_send_raw_fail(self, mock_send_message):
        """Tests that can_send_raw raises an Exception if CAN msg fails to send"""
        mock_send_message.side_effect = can.CanError
        mock_msg_id = 0
        mock_data = [0, 0, 255]
        self.assertRaises(Exception, can_send_raw, mock_msg_id, mock_data)

    @patch('cantools.database.load_file')
    def test_load_dbc_fail(self, mock_cantools_load_file):
        """Tests that load_dbc raises an Exception if no file is found"""
        mock_cantools_load_file.side_effect = can.CanError
        self.assertRaises(Exception, load_dbc, "./some-file-path")

    @patch('cantools.database.load_file')
    @patch('can_util.get_bus')
    def test_can_send_parameters(self, mock_get_bus, mock_load_file):
        """Tests accuracy of paramters passed into can_send"""
        # pylint: disable=attribute-defined-outside-init
        self.arbitration_id = None
        self.data = None

        # pylint: disable=missing-docstring
        def parameter_test(can_msg):
            self.arbitration_id = can_msg.arbitration_id
            self.data = can_msg.data

        # Creates mock object with frame_id attribute and encode function
        msg_obj = Mock()
        msg_obj.frame_id = 1
        msg_obj.encode.return_value = [1, 2]

        # Calling bus.send() triggers parameter test
        can_msg = Mock()
        can_msg.send.return_value = 3
        can_msg.send.side_effect = parameter_test

        database = Mock()

        database.get_message_by_name.return_value = msg_obj
        mock_load_file.return_value = database
        mock_get_bus.return_value = can_msg

        # dbc_database must be initialized before using can_send
        load_dbc("./some_file_path")
        can_send("some message", "vcan0", time=20)
        self.assertEqual(1, self.arbitration_id)
        self.assertEqual(bytearray(b'\x01\x02'), self.data)

    @patch('cantools.database.load_file')
    def test_can_send_fail(self, mock_load_file):
        """Tests that can_send raises an Exception if msg_obj data cannot be encoded"""
        msg_obj = Mock()
        msg_obj.frame_id = 1
        # An error is raised when msg_obj data is encoded
        msg_obj.encode.side_effect = can.CanError

        database = Mock()

        database.get_message_by_name.return_value = msg_obj
        mock_load_file.return_value = database

        # dbc_database must be initialized before using can_send
        load_dbc("./some_file_path")

        self.assertRaises(Exception, can_send, "some message")


if __name__ == '__main__':
    unittest.main()
