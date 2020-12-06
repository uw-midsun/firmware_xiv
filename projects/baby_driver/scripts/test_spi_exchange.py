"""This Modules tests methods in spi_exchange.py"""

import unittest
from unittest.mock import patch

from spi_exchange import spi_exchange
import can_util
from message_defs import BABYDRIVER_DEVICE_ID, BABYDRIVER_CAN_MESSAGE_ID

class TestSPIExchange(unittest.TestCase):
    """Test spi_exchange function"""

    @patch('can_util.send_message')
    @patch('can_util.next_message')
    def test_send_message(self, mock_next_msg, mock_send_msg):
        """Tests sending function message"""
        rx_data = spi_exchange(
            tx_bytes=bytes([1, 1, 1, 1, 1, 1, 1, 1]),
            rx_len=5,
            spi_port=2,
            spi_mode=0,
            baudrate=5000000,
            cs=(1, 1)
        )

        print(rx_data)


if __name__ == '__main__':
    unittest.main()
