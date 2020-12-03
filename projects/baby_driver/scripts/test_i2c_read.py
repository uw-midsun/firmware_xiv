"""This Module Tests methods in i2c_read.py"""
import unittest
from unittest.mock import patch

from i2c_read import i2c_read
from message_defs import BABYDRIVER_DEVICE_ID, BABYDRIVER_CAN_MESSAGE_ID

I2C_READ_COMMAND = 6
# pylint: disable=unused-argument
class TestI2CRead(unittest.TestCase):
    """Test Babydriver's i2c_read function"""

    @patch('can_util.send_message')
    @patch('can_util.next_message')
    # pylint: disable=no-self-use
    def test_i2c_read_parameters(self, mock_next_message, mock_send_message):
        """Tests parameters passed into i2c_read"""

        # Tests minimum values for address, rx_len, and reg
        mock_next_message.return_value.data = [0, 0]
        i2c_read(0, 2, 1, 3)
        i2c_read(0, 0, 1, 3)
        i2c_read(1, 2, 0, 3)
        i2c_read(1, 0, 5)
        i2c_read(1, 1, 5, 0)

        # Tests maximum values for address, rx_len, and reg
        i2c_read(0, 255, 1, 3)
        i2c_read(0, 0, 255, 3)
        i2c_read(1, 255, 5)
        i2c_read(1, 1, 5, 255)

    @patch('can_util.send_message')
    @patch('can_util.next_message')
    def test_min_values(self, mock_next_message, mock_send_message):
        """Tests minimum i2c_read parameters for can_util.send_message"""

        # Stores parameters passed into can_util.send_message
        # pylint: disable=attribute-defined-outside-init
        self.babydriver_id = None
        self.data = None
        self.channel = None
        self.msg_id = None
        self.device_id = None

        def parameter_test(babydriver_id=None,
            data=None,
            channel=None,
            msg_id=BABYDRIVER_CAN_MESSAGE_ID,
            device_id=BABYDRIVER_DEVICE_ID
        ):
            self.babydriver_id = babydriver_id
            self.data = data
            self.channel = channel
            self.msg_id = msg_id
            self.device_id = device_id

        mock_send_message.side_effect = parameter_test
        mock_next_message.return_value.data = [0, 0]
        # Low port value
        i2c_read(0, 2, 1, 3)
        self.assertEqual(I2C_READ_COMMAND, self.babydriver_id)
        self.assertEqual([0, 2, 1, 1, 3], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)
        # Min address value
        i2c_read(0, 0, 1, 3)
        self.assertEqual(I2C_READ_COMMAND, self.babydriver_id)
        self.assertEqual([0, 0, 1, 1, 3], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)
        # Min rx_len value
        i2c_read(1, 2, 0, 3)
        self.assertEqual(I2C_READ_COMMAND, self.babydriver_id)
        self.assertEqual([1, 2, 0, 1, 3], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)
        # None reg value
        i2c_read(1, 0, 5)
        self.assertEqual(I2C_READ_COMMAND, self.babydriver_id)
        self.assertEqual([1, 0, 5, 0, 0], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)
        # Min reg value
        i2c_read(1, 1, 5, 0)
        self.assertEqual(I2C_READ_COMMAND, self.babydriver_id)
        self.assertEqual([1, 1, 5, 1, 0], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)

    @patch('can_util.send_message')
    @patch('can_util.next_message')
    def test_max_values(self, mock_next_message, mock_send_message):
        """Tests maximum i2c_read parameters for can_util.send_message"""

        # Stores parameters passed into can_util.send_message
        # pylint: disable=attribute-defined-outside-init
        self.babydriver_id = None
        self.data = None
        self.channel = None
        self.msg_id = None
        self.device_id = None

        def parameter_test(babydriver_id=None,
            data=None,
            channel=None,
            msg_id=BABYDRIVER_CAN_MESSAGE_ID,
            device_id=BABYDRIVER_DEVICE_ID
        ):
            self.babydriver_id = babydriver_id
            self.data = data
            self.channel = channel
            self.msg_id = msg_id
            self.device_id = device_id

        mock_send_message.side_effect = parameter_test
        mock_next_message.return_value.data = [0, 0]
        # High port value
        i2c_read(1, 2, 1, 3)
        self.assertEqual(I2C_READ_COMMAND, self.babydriver_id)
        self.assertEqual([1, 2, 1, 1, 3], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)
        # Max address value
        i2c_read(0, 255, 1, 3)
        self.assertEqual(I2C_READ_COMMAND, self.babydriver_id)
        self.assertEqual([0, 255, 1, 1, 3], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)
        # Max rx_len value
        i2c_read(0, 3, 255, 3)
        self.assertEqual(I2C_READ_COMMAND, self.babydriver_id)
        self.assertEqual([0, 3, 255, 1, 3], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)
        # None reg value
        i2c_read(1, 255, 5)
        self.assertEqual(I2C_READ_COMMAND, self.babydriver_id)
        self.assertEqual([1, 255, 5, 0, 0], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)
        # Max reg value
        i2c_read(1, 1, 5, 255)
        self.assertEqual(I2C_READ_COMMAND, self.babydriver_id)
        self.assertEqual([1, 1, 5, 1, 255], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)

    @patch('can_util.send_message')
    @patch('can_util.next_message')
    def test_fail_conditions(self, mock_next_message, mock_send_message):
        """Tests fail conditions"""

        # Tests invalid port number
        self.assertRaises(ValueError, i2c_read, -1, 0, 1, 2)
        # Test invalid address
        self.assertRaises(ValueError, i2c_read, 1, -1, 1, 2)
        self.assertRaises(ValueError, i2c_read, 0, 256, 1, 2)
        # Test invalid rx_len
        self.assertRaises(ValueError, i2c_read, 1, 0, -1, 2)
        self.assertRaises(ValueError, i2c_read, 0, 0, 256, 2)
        # Test invalid reg
        self.assertRaises(ValueError, i2c_read, 1, 0, 1, -1)
        self.assertRaises(ValueError, i2c_read, 0, 0, 1, 256)
        # Test invalid values with no reg specified (reg = None)
        self.assertRaises(ValueError, i2c_read, 1, 0, -1)
        self.assertRaises(ValueError, i2c_read, 0, 256, 2)
        self.assertRaises(ValueError, i2c_read, 3, 1, 2)
        # Tests failing status code from can_util.next_message
        mock_next_message.return_value.data = [0, 1]
        self.assertRaises(Exception, i2c_read, 0, 0, 0, 0)

if __name__ == "__main__":
    unittest.main()
