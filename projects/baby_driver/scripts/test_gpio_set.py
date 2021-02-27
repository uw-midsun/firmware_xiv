"""This Module Tests methods in gpio_set.py"""
import unittest
from unittest.mock import patch

from gpio_set import gpio_set, NUM_PINS_PER_PORT
from gpio_port import GpioPort
from message_defs import BABYDRIVER_DEVICE_ID, BABYDRIVER_CAN_MESSAGE_ID


# pylint: disable=unused-argument
class TestGpioSet(unittest.TestCase):
    """Test gpio_set function"""

    @patch('can_util.send_message')
    @patch('can_util.next_message')
    # pylint: disable=no-self-use
    def test_gpio_set_parameters(self, mock_next_message, mock_send_message):
        """Tests parameters passed into gpio_set"""

        # Tests minimum values for port, pin, and state
        mock_next_message.return_value.data = [0, 0]
        gpio_set('A', 6, 1)
        gpio_set('C', 0, 0)
        gpio_set(GpioPort.A, 5, 1)
        gpio_set(GpioPort.A, 0, 0)

        # Tests maximum values for port, pin, and state
        gpio_set('F', 5, 0)
        gpio_set(GpioPort.E, 8, 1)
        gpio_set(GpioPort.C, 15, 1)
        gpio_set('F', 15, 1)

    @patch('can_util.send_message')
    @patch('can_util.next_message')
    def test_send_message(self, mock_next_message, mock_send_message):
        """Tests accuracy of parameters passed into can_util.send_message"""

        # Stores parameters passed into can_util.send_message
        # pylint: disable=attribute-defined-outside-init
        self.babydriver_id = None
        self.data = None
        self.channel = None
        self.msg_id = None
        self.device_id = None

        def parameter_test(
            babydriver_id=None,
            data=None,
            channel=None,
            msg_id=BABYDRIVER_CAN_MESSAGE_ID,
            device_id=BABYDRIVER_DEVICE_ID,
        ):

            self.babydriver_id = babydriver_id
            self.data = data
            self.channel = channel
            self.msg_id = msg_id
            self.device_id = device_id

        # Tests low parameters for can_util.send_message
        mock_send_message.side_effect = parameter_test
        mock_next_message.return_value.data = [0, 0]
        gpio_set(0, 10, 1)
        self.assertEqual(1, self.babydriver_id)
        self.assertEqual([0, 10, 1], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)

        gpio_set(3, 0, 0)
        self.assertEqual(1, self.babydriver_id)
        self.assertEqual([3, 0, 0], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)

        # Tests high parameters for can_util.send_message
        gpio_set(GpioPort.NUM_GPIO_PORTS - 1, 4, 0)
        self.assertEqual(1, self.babydriver_id)
        self.assertEqual([5, 4, 0], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)

        gpio_set(3, NUM_PINS_PER_PORT - 1, 1)
        self.assertEqual(1, self.babydriver_id)
        self.assertEqual([3, 15, 1], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)

    @patch('can_util.send_message')
    @patch('can_util.next_message')
    def test_fail_conditions(self, mock_next_message, mock_send_message):
        """Tests fail conditions"""

        # Tests fail condition from gpio_set parameter input
        self.assertRaises(AttributeError, gpio_set, 'G', 0, 0)
        self.assertRaises(ValueError, gpio_set, GpioPort.NUM_GPIO_PORTS, 6, 1)
        self.assertRaises(AttributeError, gpio_set, '1', 5, 0)
        self.assertRaises(ValueError, gpio_set, GpioPort.A, NUM_PINS_PER_PORT, 0)
        self.assertRaises(ValueError, gpio_set, 'f', -1, 1)
        self.assertRaises(ValueError, gpio_set, GpioPort.D, 0, -1)
        self.assertRaises(ValueError, gpio_set, 'C', 15, 2)

        # Tests failing status code from can_util.next_message
        mock_next_message.return_value.data = [0, 1]
        self.assertRaises(Exception, gpio_set, 'A', 0, 0)
        self.assertRaises(Exception, gpio_set, GpioPort.F, 15, 1)


if __name__ == "__main__":
    unittest.main()
