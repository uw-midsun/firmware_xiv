"""This Module Tests methods in gpio_set.py"""
import unittest
from unittest.mock import patch

from gpio_set import gpio_set
from gpio_port import GpioPort

class TestGpioSet(unittest.TestCase):
    """Test gpio_set function"""

    @patch('can_util.send_message')
    @patch('can_util.next_message')
    def test_low_bound(self, mock_send_message, mock_next_message):
        """Tests minimum values for pin and port"""
        mock_send_message.side_effect = None
        mock_next_message.side_effect = None
        self.assertEqual(True, gpio_set(GpioPort.C, 0, 1))
        self.assertEqual(True, gpio_set(GpioPort.A, 5, 0))
        self.assertEqual(True, gpio_set(GpioPort.A, 0, 0))

    @patch('can_util.send_message')
    @patch('can_util.next_message')
    def test_high_bound(self, mock_send_message, mock_next_message):
        """Tests maximum values for pin and port"""
        mock_send_message.side_effect = None
        mock_next_message.side_effect = None
        self.assertEqual(True, gpio_set(GpioPort.D, 15, 0))
        self.assertEqual(True, gpio_set(GpioPort.NUM_GPIO_PORTS, 3, 1))
        self.assertEqual(True, gpio_set(GpioPort.NUM_GPIO_PORTS, 15, 1))

    @patch('can_util.send_message')
    @patch('can_util.next_message')
    def test_fail_conditions(self, mock_send_message, mock_next_message):
        """Tests fail conditions"""

        #Tests fail conditions from can_util.send_message
        mock_send_message.side_effect = ValueError
        mock_next_message.side_effect = None
        self.assertEqual(False, gpio_set(GpioPort.A, 0, 0))
        self.assertEqual(False, gpio_set(GpioPort.NUM_GPIO_PORTS, 15, 1))

        #Tests fail condition from can_util.next_message
        mock_send_message.side_effect = None
        mock_next_message.side_effect = ValueError
        self.assertEqual(False, gpio_set(GpioPort.A, 0, 0))
        self.assertEqual(False, gpio_set(GpioPort.NUM_GPIO_PORTS, 15, 1))

        #Tests fail condition from gpio_set parameter input
        mock_send_message.side_effect = None
        mock_next_message.side_effect = None
        self.assertEqual(False, gpio_set(7, 5, 0))
        self.assertEqual(False, gpio_set(GpioPort.B, 16, 0))
        self.assertEqual(False, gpio_set(GpioPort.C, -1, 1))
        self.assertEqual(False, gpio_set(GpioPort.D, 5, 2))
        self.assertEqual(False, gpio_set(GpioPort.E, 8, -1))


if __name__ == "__main__":
    unittest.main()
