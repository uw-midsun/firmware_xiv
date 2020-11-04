"""This Module Tests methods in gpio_get.py"""
# pylint: disable=unused-import
import unittest
from unittest.mock import patch

from gpio_get import gpio_get
from gpio_port import GpioPort
from can_util import Message
from message_defs import BABYDRIVER_DEVICE_ID, BABYDRIVER_CAN_MESSAGE_ID

class TestGpioGet(unittest.TestCase):
    """Test Babydriver's gpio_get function"""

    @patch('can_util.send_message')
    @patch('can_util.next_message')
    def test_low_bound(self, mock_next_message, mock_send_message):
        '''Tests minimum values for port and pin'''
        gpio_pin_data = [1]
        gpio_pin_msg = Message(
            message_id=BABYDRIVER_CAN_MESSAGE_ID,
            device_id=BABYDRIVER_DEVICE_ID,
            data=gpio_pin_data,
        )

        status_data = [0]
        status_msg = Message(
            message_id=BABYDRIVER_CAN_MESSAGE_ID,
            device_id=BABYDRIVER_DEVICE_ID,
            data=status_data,
        )
        mock_send_message.side_effect = None
        mock_next_message.side_effect = [gpio_pin_msg, status_msg]

        self.assertEqual(True, gpio_get(GpioPort.A, 0))

        gpio_pin_data = [0]
        gpio_pin_msg = Message(
            message_id=BABYDRIVER_CAN_MESSAGE_ID,
            device_id=BABYDRIVER_DEVICE_ID,
            data=gpio_pin_data,
        )
        mock_next_message.side_effect = [gpio_pin_msg, status_msg]
        self.assertEqual(False, gpio_get(GpioPort.A, 0))

    @patch('can_util.send_message')
    @patch('can_util.next_message')
    def test_high_bound(self, mock_next_message, mock_send_message):
        '''Tests maximum values for port and pin'''
        gpio_pin_data = [1]
        gpio_pin_msg = Message(
            message_id=BABYDRIVER_CAN_MESSAGE_ID,
            device_id=BABYDRIVER_DEVICE_ID,
            data=gpio_pin_data,
        )

        status_data = [0]
        status_msg = Message(
            message_id=BABYDRIVER_CAN_MESSAGE_ID,
            device_id=BABYDRIVER_DEVICE_ID,
            data=status_data,
        )
        mock_send_message.side_effect = None
        mock_next_message.side_effect = [gpio_pin_msg, status_msg]

        self.assertEqual(True, gpio_get(GpioPort.F, 15))

        gpio_pin_data = [0]
        gpio_pin_msg = Message(
            message_id=BABYDRIVER_CAN_MESSAGE_ID,
            device_id=BABYDRIVER_DEVICE_ID,
            data=gpio_pin_data,
        )
        mock_next_message.side_effect = [gpio_pin_msg, status_msg]
        self.assertEqual(False, gpio_get(GpioPort.F, 15))


    @patch('can_util.send_message')
    @patch('can_util.next_message')
    def test_fail_conditions(self, mock_next_message, mock_send_message):
        '''Tests fail conditions'''
        gpio_pin_data = [1]
        gpio_pin_msg = Message(
            message_id=BABYDRIVER_CAN_MESSAGE_ID,
            device_id=BABYDRIVER_DEVICE_ID,
            data=gpio_pin_data,
        )

        status_data = [1]
        status_msg = Message(
            message_id=BABYDRIVER_CAN_MESSAGE_ID,
            device_id=BABYDRIVER_DEVICE_ID,
            data=status_data,
        )
        mock_send_message.side_effect = None
        mock_next_message.side_effect = [gpio_pin_msg, status_msg]

        self.assertRaises(Exception, gpio_get, GpioPort.A, 0)

        status_data = [0]
        status_msg = Message(
            message_id=BABYDRIVER_CAN_MESSAGE_ID,
            device_id=BABYDRIVER_DEVICE_ID,
            data=status_data,
        )

        #invalid port fail
        mock_next_message.side_effect = [gpio_pin_msg, status_msg]
        self.assertRaises(ValueError, gpio_get, -1, 0)
        mock_next_message.side_effect = [gpio_pin_msg, status_msg]
        self.assertRaises(ValueError, gpio_get, 16, 0)

        #invalid pin fail
        mock_next_message.side_effect = [gpio_pin_msg, status_msg]
        self.assertRaises(ValueError, gpio_get, GpioPort.A, -1)
        mock_next_message.side_effect = [gpio_pin_msg, status_msg]
        self.assertRaises(ValueError, gpio_get, GpioPort.A, 16)


if __name__ == '__main__':
    unittest.main()
