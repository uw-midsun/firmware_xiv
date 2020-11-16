"""This Module Tests methods in adc_read.py"""
import unittest
from unittest.mock import patch

from adc_read import adc_read, NUM_PINS_PER_PORT
from can_util import can_pack, Message
from gpio_port import GpioPort
from message_defs import BabydriverMessageId, BABYDRIVER_CAN_MESSAGE_ID

OK_STATUS = 0

class TestADCRead(unittest.TestCase):
    """Test Babydriver's adc_read function"""

    @patch('can_util.send_message')
    @patch('can_util.next_message')
    def test_check_message_format(self, mock_next_message, mock_send_message):
        """Tests that CAN messages are passed with the correct parameters"""

        # pylint: disable=attribute-defined-outside-init
        # Parameters to be sent to the firmware project
        self.msg_id = None
        self.port = None
        self.pin = None
        self.raw = None

        def parameter_test(msg_id=BABYDRIVER_CAN_MESSAGE_ID, port=1, pin=1, raw=1):
            self.msg_id = msg_id
            self.port = port
            self.pin = pin
            self.raw = raw

        mock_send_message.side_effect = parameter_test

        # Set mock returns for 2 calls of next_message
        data_msg = [BabydriverMessageId.ADC_READ_DATA, 1, 1]
        status_msg = [BabydriverMessageId.ADC_READ_DATA, OK_STATUS]
        mock_next_message.side_effect = (Message(data=data_msg), Message(data=status_msg))

        adc_read(1, 1, False)
        self.assertEqual(self.port, 1)
        self.assertEqual(self.pin, 1)
        self.assertEqual(self.raw, 1)

    @patch('can_util.send_message')
    @patch('can_util.next_message')
    def test_check_values(self, mock_next_message, mock_send_message):
        """Tests the ADC's expected return value"""

        mock_send_message.return_value = None

        # Set mock returns for 2 calls of next_message
        data_msg = [BabydriverMessageId.ADC_READ_DATA, 1, 1]
        status_msg = [BabydriverMessageId.ADC_READ_DATA, OK_STATUS]
        mock_next_message.side_effect = (Message(data=data_msg), Message(data=status_msg))
        
        converted_read = int(adc_read(GpioPort.A, 0, False))

        result_low = 1
        result_high = 1
        expected_result = int((result_high << 8) | result_low)
        self.assertEqual(converted_read, expected_result)

    @patch('can_util.send_message')
    @patch('can_util.next_message')
    def test_fail_conditions(self, mock_next_message, mock_send_message):
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
        bad_send_data = [
            (0, 1),
            (0, 1),
            (0, 1),
        ]
        bad_send_data = can_pack(bad_send_data)
        mock_send_message.return_value = Message(data=bad_send_data)
        self.assertRaises(Exception, adc_read, GpioPort.A, 0, 0)

        # Tests fail condition for can_util.next_message, in index 1
        bad_next_data = [
            (0, 1),
            (1, 1),
            (0, 1),
        ]
        bad_next_data = can_pack(bad_next_data)
        mock_next_message.return_value = Message(data=bad_next_data)
        self.assertRaises(Exception, adc_read, GpioPort.A, 0, 0)

if __name__ == '__main__':
    unittest.main()
