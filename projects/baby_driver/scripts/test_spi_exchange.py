"""This Modules tests methods in spi_exchange.py"""

import unittest
from unittest.mock import patch

from spi_exchange import spi_exchange
from gpio_port import GpioPort
from can_util import Message
from message_defs import BABYDRIVER_DEVICE_ID, BABYDRIVER_CAN_MESSAGE_ID, BabydriverMessageId

OK_STATUS = 0
FAILING_STATUS = 1

class TestSpiExchange(unittest.TestCase):
    """Test spi_exchange function"""

    @patch('can_util.send_message')
    @patch('can_util.next_message')
    def test_send_message(self, mock_next_message, mock_send_message):
        """Tests sending function message"""

        # Stores parameters passed into can_util.send_message
        # pylint: disable=attribute-defined-outside-init
        self.babydriver_id = None
        self.data = None
        self.channel = None
        self.msg_id = None
        self.device_id = None

        def send_msg_test(
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

        mock_send_message.side_effect = send_msg_test

        data_msg = [BabydriverMessageId.SPI_EXCHANGE_RX_DATA, 0, 0, 0, 0, 0, 0, 0]
        status_msg = [BabydriverMessageId.STATUS, OK_STATUS]

        # Normal test
        mock_next_message.side_effect = (Message(data=data_msg), Message(data=status_msg))
        self.assertEqual(spi_exchange(
            tx_bytes=[1, 1, 1, 1, 1, 1, 1, 1, 1, 1],  # 10 bits
            rx_len=5,
            spi_port=1,
            spi_mode=0,
            baudrate=5000000,
            cs=("A", 1),
        ), [0, 0, 0, 0, 0])

        # Test default values
        mock_next_message.side_effect = (Message(data=data_msg), Message(data=status_msg))
        self.assertEqual(spi_exchange(
            [1, 1, 1, 1, 1, 1, 1, 1, 1, 1], 5
        ), [0, 0, 0, 0, 0])

        # len(tx_bytes) = 255
        mock_next_message.side_effect = (Message(data=data_msg), Message(data=status_msg))
        self.assertEqual(spi_exchange(
            [1] * 255, 5
        ), [0, 0, 0, 0, 0])

        # 0 rx_len
        mock_next_message.side_effect = (Message(data=data_msg), Message(data=status_msg))
        self.assertEqual(spi_exchange(
            [1, 1, 1, 1, 1, 1, 1, 1, 1, 1], 0, 2, 0, 5000000, ("A", 1),
        ), [])

        # Test rx_len < 7
        mock_next_message.side_effect = (Message(data=data_msg), Message(data=status_msg))
        self.assertEqual(spi_exchange(
            [1, 1, 1, 1, 1, 1, 1, 1, 1, 1], 6
        ), [0, 0, 0, 0, 0, 0])

        # Test rx_len equal a multiple of 7
        mock_next_message.side_effect = (Message(data=data_msg), Message(data=status_msg))
        self.assertEqual(spi_exchange(
            [1, 1, 1, 1, 1, 1, 1, 1, 1, 1], 7
        ), [0, 0, 0, 0, 0, 0, 0])

        # Test rx_len > 7
        mock_next_message.side_effect = (Message(data=data_msg), Message(data=data_msg), \
                                        Message(data=status_msg))
        self.assertEqual(spi_exchange(
            [1, 1, 1, 1, 1, 1, 1, 1, 1, 1], 8
        ), [0, 0, 0, 0, 0, 0, 0, 0])

        # Test rx_len = 255
        mock_next_message.side_effect = (Message(data=data_msg),) * 37 + (Message(data=status_msg),)
        self.assertEqual(spi_exchange(
            [1, 1, 1, 1, 1, 1, 1, 1, 1, 1], 255
        ), [0] * 255)

        # Test multiple messages, low rx_len
        mock_next_message.side_effect = (Message(data=data_msg), Message(data=data_msg), \
                                        Message(data=status_msg))
        self.assertEqual(spi_exchange(
            [1, 1, 1, 1, 1, 1, 1, 1, 1, 1], 3
        ), [0, 0, 0])


    @patch('can_util.send_message')
    @patch('can_util.next_message')
    # pylint: disable=unused-argument
    def test_fail_conditions(self, mock_next_message, mock_send_message):
        """Tests fail conditions"""

        # Invalid spi_port
        self.assertRaises(ValueError, spi_exchange, [0], 1, 'G', 0, 50)
        self.assertRaises(ValueError, spi_exchange, [0], 1, GpioPort.NUM_GPIO_PORTS, 0, 50)
        self.assertRaises(ValueError, spi_exchange, [0], 1, -1, 0, 50)

        # Invalid spi_mode
        self.assertRaises(ValueError, spi_exchange, [0], 1, 1, -1, 50)
        self.assertRaises(ValueError, spi_exchange, [0], 1, 1, 4, 50)

        # Invalid number of tx_bytes
        self.assertRaises(ValueError, spi_exchange, [0] * 256, 1, 1, 2, 50)

        # Invalid rx_len
        self.assertRaises(ValueError, spi_exchange, [0], -1, 1, 2, 50)
        self.assertRaises(ValueError, spi_exchange, [0], 256, 1, 2, 50)

        # Invalid cs_port
        self.assertRaises(ValueError, spi_exchange, [0], 1, 1, 2, 50, (-1, 2))
        self.assertRaises(ValueError, spi_exchange, [0], 1, 1, 2, 50, (7, 2))
        self.assertRaises(AttributeError, spi_exchange, [0], 1, 1, 2, 50, ("G", 2))

        # Invalid cs_pin
        self.assertRaises(ValueError, spi_exchange, [0], 1, GpioPort.A, 2, 50, (1, -1))
        self.assertRaises(ValueError, spi_exchange, [0], 1, GpioPort.A, 2, 50, (1, 17))

        # Failing status code
        data_msg = [BabydriverMessageId.SPI_EXCHANGE_RX_DATA, 0, 0, 0, 0, 0, 0, 0]
        status_msg = [BabydriverMessageId.STATUS, FAILING_STATUS]
        mock_next_message.side_effect = (Message(data=data_msg), Message(data=status_msg))
        self.assertRaises(Exception, spi_exchange, [0], 1, GpioPort.A, 3, 50)


if __name__ == '__main__':
    unittest.main()
