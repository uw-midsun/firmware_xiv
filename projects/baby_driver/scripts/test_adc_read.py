"""This Module Tests methods in adc_read.py"""
import unittest
from unittest.mock import patch

from adc_read import adc_read, NUM_PINS_PER_PORT
from can_util import Message
from gpio_port import GpioPort

RAW_MAX = 4095
CONVERTED_MAX = 3000

class TestADCRead(unittest.TestCase):
    """Test Babydriver's adc_read function"""

    @patch('can_util.send_message')
    @patch('can_util.next_message')
    def test_check_range_pin(self):
        """Tests Maximum values"""
        self.assertLessEqual(adc_read(GpioPort.A, 0, True), RAW_MAX)
        self.assertLessEqual(adc_read(GpioPort.A, 1, True), RAW_MAX)
        self.assertLessEqual(adc_read(GpioPort.A, 2, True), RAW_MAX)
        self.assertLessEqual(adc_read(GpioPort.A, 0, False), CONVERTED_MAX)
        self.assertLessEqual(adc_read(GpioPort.A, 1, False), CONVERTED_MAX)
        self.assertLessEqual(adc_read(GpioPort.A, 2, False), CONVERTED_MAX)

    @patch('can_util.send_message')
    @patch('can_util.next_message')
    def test_fail_conditions(self, mock_send_message, mock_next_message):
        """Tests fail conditions"""

        # Tests fail conditions for port parameter
        self.assertRaises(ValueError, adc_read, GpioPort.NUM_GPIO_PORTS, 1, 0)
        self.assertRaises(ValueError, adc_read, -1, 1, 0)
        # Tests fail conditions for pin parameter
        self.assertRaises(ValueError, adc_read, GpioPort.A, NUM_PINS_PER_PORT, 0)
        self.assertRaises(ValueError, adc_read, GpioPort.A, -1, 0)
        # Tests fail conditions for raw parameter
        self.assertRaises(ValueError, adc_read, GpioPort.A, 1, -1)

        # Tests fail conditions for can_util.send_message, in index 0
        mock_send_message.side_effect = Message([-1])
        self.assertRaises(Exception, adc_read, GpioPort.A, 0, 0)

        # Tests fail condition for can_util.next_message, in index 1
        mock_next_message.side_effect = Message([0, -1])
        self.assertRaises(Exception, adc_read, GpioPort.A, 0, 0)

if __name__ == '__main__':
    unittest.main()
