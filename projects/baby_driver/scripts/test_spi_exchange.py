"""This Modules tests methods in spi_exchange.py"""

import unittest
from unittest.mock import patch

from spi_exchange import spi_exchange
from gpio_port import GpioPort
from can_util import Message
from message_defs import BABYDRIVER_DEVICE_ID, BABYDRIVER_CAN_MESSAGE_ID, BabydriverMessageId

# TODO
# shorten spi exchange calls without param names

FAILING_STATUS = 1
class TestSPIExchange(unittest.TestCase):
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
        # self.read_status = [BabydriverMessageId.STATUS, 0]

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

        # Tests parameters for can_util.send_message
        mock_send_message.side_effect = parameter_test
        # mock data sent back from firmware project, each message sends 7 bits
        mock_next_message.return_value.data = [0, 0, 0, 0, 0, 0, 0]

        # Normal test
        self.assertEqual(spi_exchange(
            tx_bytes=[1, 1, 1, 1, 1, 1, 1, 1, 1, 1],  # 10 bits
            rx_len=5,
            spi_port="B",
            spi_mode=0,
            baudrate=5000000,
            cs=("A", 1),
        ), [0, 0, 0, 0, 0])
        # self.assertEqual(BabydriverMessageId.SPI_EXCHANGE_METADATA_1, self.babydriver_id) TODO returns TX_DATA for some reason...
        #self.assertEqual([12, 1, 1, 1, 0, 0, 0, 0], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)

        # Test with spi_port as an int value
        self.assertEqual(spi_exchange(
            tx_bytes=[1, 1, 1, 1, 1, 1, 1, 1, 1, 1],  # 10 bits
            rx_len=5,
            spi_port=2,
            spi_mode=0,
            baudrate=5000000,
            cs=(1, 1),
        ), [0, 0, 0, 0, 0])
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)

        # Test default values
        self.assertEqual(spi_exchange(
            tx_bytes=[1, 1, 1, 1, 1, 1, 1, 1, 1, 1],  # 10 bits
            rx_len=5,
        ), [0, 0, 0, 0, 0])
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)

        # 0 rx_len
        self.assertEqual(spi_exchange(
            tx_bytes=[1, 1, 1, 1, 1, 1, 1, 1, 1, 1],  # 10 bits
            rx_len=0,
            spi_port="B",
            spi_mode=0,
            baudrate=5000000,
            cs=("A", 1),
        ), [])
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)

        # Test rx_len < 7
        self.assertEqual(spi_exchange(
            tx_bytes=[1, 1, 1, 1, 1, 1, 1, 1, 1, 1],  # 10 bits
            rx_len=6,
        ), [0, 0, 0, 0, 0, 0])
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)

        # Test rx_len equal a multiple of 7
        self.assertEqual(spi_exchange(
            tx_bytes=[1, 1, 1, 1, 1, 1, 1, 1, 1, 1],  # 10 bits
            rx_len=7,
        ), [0, 0, 0, 0, 0, 0, 0])
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)

        # Test rx_len > 7
        self.assertEqual(spi_exchange(
            tx_bytes=[1, 1, 1, 1, 1, 1, 1, 1, 1, 1],  # 10 bits
            rx_len=8,
        ), [0, 0, 0, 0, 0, 0, 0, 0])
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)

    @patch('can_util.send_message')
    @patch('can_util.next_message')
    def test_fail_conditions(self, mock_next_message, mock_send_message):
        """Tests fail conditions"""

        # Invalid spi_port
        self.assertRaises(AttributeError, spi_exchange, [0], 1, 'G', 0, 50)
        self.assertRaises(ValueError, spi_exchange, [0], 1, GpioPort.NUM_GPIO_PORTS, 0, 50)
        self.assertRaises(AttributeError, spi_exchange, [0], 1, '2', 0, 50)
        self.assertRaises(ValueError, spi_exchange, [0], 1, -1, 0, 50)

        # Invalid spi_mode
        self.assertRaises(ValueError, spi_exchange, [0], 1, GpioPort.A, -1, 50)
        self.assertRaises(ValueError, spi_exchange, [0], 1, GpioPort.A, 0, 50)
        self.assertRaises(ValueError, spi_exchange, [0], 1, GpioPort.A, 4, 50)

        # Invalid rx_len
        self.assertRaises(ValueError, spi_exchange, [0], -1, GpioPort.A, 2, 50)

        # Invalid cs_port
        self.assertRaises(ValueError, spi_exchange, [0], 1, GpioPort.A, 2, 50, (-1, 2))
        self.assertRaises(ValueError, spi_exchange, [0], 1, GpioPort.A, 2, 50, (7, 2))
        self.assertRaises(ValueError, spi_exchange, [0], 1, GpioPort.A, 2, 50, ("G", 2))

        # Invalid cs_pin
        self.assertRaises(ValueError, spi_exchange, [0], 1, GpioPort.A, 2, 50, (1, -1))
        self.assertRaises(ValueError, spi_exchange, [0], 1, GpioPort.A, 2, 50, (1, 17))

        # Failing status code
        mock_next_message.return_value.data = [0, 0, 0] # bad return data
        self.assertRaises(Exception, spi_exchange, [0], 1, GpioPort.A, 3, 50)
        # Set mock returns for 2 calls of next_message
        data_msg = [BabydriverMessageId.SPI_EXCHANGE_RX_DATA, 0, 0, 0, 0, 0, 0, 0]
        status_msg = [BabydriverMessageId.STATUS, FAILING_STATUS]
        # mock_next_message.return_value.data = [0, 0, 0, 0, 0, 0, 0] # normal return data
        mock_next_message.side_effect = (Message(data=data_msg), Message(data=status_msg))
        self.assertRaises(Exception, spi_exchange, [0], 1, GpioPort.A, 3, 50)


if __name__ == '__main__':
    unittest.main()
