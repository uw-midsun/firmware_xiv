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
    def test_send_message(self, mock_next_message, mock_send_message):
        """Tests sending function message"""
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
        
        rx_data = spi_exchange(
            tx_bytes=[1, 1, 1, 1, 1, 1, 1, 1, 1, 1], # 10 bits
            rx_len=5,
            spi_port=2,
            spi_mode=0,
            baudrate=5000000,
            cs=(1, 1),
        )

        print("test rx_data")
        print(rx_data)


if __name__ == '__main__':
    unittest.main()
