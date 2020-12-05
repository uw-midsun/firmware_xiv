"""This module tests methods in i2c_write.py"""
import unittest
from unittest.mock import patch

from i2c_write import i2c_write
from message_defs import BABYDRIVER_DEVICE_ID, BABYDRIVER_CAN_MESSAGE_ID
from message_defs import BabydriverMessageId

#pylint: disable=unused-argument
class TestI2CWrite(unittest.TestCase):
    """Test i2c_write function"""

    @patch('can_util.send_message')
    @patch('can_util.next_message')
    #pylint: disable =no-self-use

    def test_i2c_write_parameters(self, mock_next_messaage, mock_send_message):
        """Tests parameters passed into i2c_write"""

        # Tests minimum values for port, address, tx_bytes and reg
        mock_next_messaage.return_value.data = [0,0]
        i2c_write(0,1,[7, 5, 12],1)
        i2c_write(1,0,[7, 5, 12],1)
        i2c_write(1,1,[0],1)
        i2c_write(1,1,[7, 5, 12],0)
        i2c_write(1,1,[7, 5, 12])

        # Tests maximum values for port, address, tx_bytes and reg
        i2c_write(1,1,[7, 5, 12],1)
        i2c_write(0,255,[7, 5, 12],1)
        i2c_write(0,1,[255, 255, 255],1)
        i2c_write(0,1,[7, 5, 12],255)

    @patch('can_util.send_message')
    @patch('can_util.next_message')

    def test_send_message_min(self, mock_next_message, mock_send_message):
        """Tests accuracy of minimum parameters passed into can_util.send_message"""

        # Stores parameters passed into can_util.send_message
        # pylint: disable=attribute-defined-outside-init
        self.babydriver_id = None
        self.data = None
        self.channel = None
        self.msg_id = None
        self.device_id = None

        def test_parameter(
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
        mock_send_message.side_effect = test_parameter
        mock_next_message.return_value.data = [0,0]

        # Tests for min value of port 
        i2c_write(0, 10, [0], 1)
        self.assertEqual(BabydriverMessageId.I2C_WRITE_DATA, self.babydriver_id)
        self.assertEqual([0], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)

        # Tests for min value of address
        i2c_write(1, 0, [0,0,0], 1)
        self.assertEqual(BabydriverMessageId.I2C_WRITE_DATA, self.babydriver_id)
        self.assertEqual([0,0,0], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)

        # Tests for min value of tx_bytes
        i2c_write(1, 10, [0], 0)
        self.assertEqual(BabydriverMessageId.I2C_WRITE_DATA, self.babydriver_id)
        self.assertEqual([0], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)

        # Tests for min value of reg
        i2c_write(1, 10, [7, 5, 12, 14], 0)
        self.assertEqual(BabydriverMessageId.I2C_WRITE_DATA, self.babydriver_id)
        self.assertEqual([7, 5, 12, 14], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)

        # Tests for if reg=None 
        i2c_write(1, 10, [7, 5, 12, 14])
        self.assertEqual(BabydriverMessageId.I2C_WRITE_DATA, self.babydriver_id)
        self.assertEqual([7, 5, 12, 14], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)

    @patch('can_util.send_message')
    @patch('can_util.next_message')
    def test_send_message_max(self, mock_next_message, mock_send_message):
        """Tests accuracy of maximum parameters passed into can_util.send_message"""
        # Stores parameters passed into can_util.send_message
        # pylint: disable=attribute-defined-outside-init
        self.babydriver_id = None
        self.data = None
        self.channel = None
        self.msg_id = None
        self.device_id = None

        def test_parameter(
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

            mock_send_message.side_effect = test_parameter
            mock_next_message.return_value.data = [0,0]

        # Tests high parameters for can_util.send_message

        # Tests high value for port
        i2c_write(1, 10, [0], 1)
        self.assertEqual(BabydriverMessageId.I2C_WRITE_DATA, self.babydriver_id)
        self.assertEqual([0], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)
        
        # Tests high value for address
        i2c_write(1, 255, [7, 5, 12, 7, 5, 12, 7, 255, 255, 255, 255, 255, 255, 255], 1)
        self.assertEqual(BabydriverMessageId.I2C_WRITE_DATA, self.babydriver_id)
        self.assertEqual([255, 255, 255, 255, 255, 255, 255], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)
        
        # Tests high value for tx_bytes
        i2c_write(1, 10, [255,255,255], 1)
        self.assertEqual(BabydriverMessageId.I2C_WRITE_DATA, self.babydriver_id)
        self.assertEqual([255, 255, 255], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)

        # Tests high value for reg
        i2c_write(1, 10, [7, 5, 12], 255)
        self.assertEqual(BabydriverMessageId.I2C_WRITE_DATA, self.babydriver_id)
        self.assertEqual([7, 5, 12], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)

        # Tests for len(tx_bytes) == 255
        testlist = [1] * 255
        i2c_write(1, 10, testlist, 1)
        self.assertEqual(BabydriverMessageId.I2C_WRITE_DATA, self.babydriver_id)
        self.assertEqual(testlist, self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)

        # Tests for if len(tx_bytes) is greater than 7 but not divisible by 7
        i2c_write(1, 10, [7, 5, 12, 7, 5, 12, 7, 5, 12], 255)
        self.assertEqual(BabydriverMessageId.I2C_WRITE_DATA, self.babydriver_id)
        self.assertEqual([7, 5, 12, 7, 5, 12, 7, 5, 12], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)

    @patch('can_util.send_message')
    @patch('can_util.next_message')

    def test_fail_conditions(self, mock_next_message, mock_send_message):
        """Tests fail conditions"""

        # Tests fail condition for i2c_write parameter input
        self.assertRaises(ValueError,i2c_write, 2, 0, [0], 1)
        self.assertRaises(ValueError,i2c_write, 1, 256, [0], 1)
        self.assertRaises(ValueError,i2c_write, 1, -1, [0], 1)
        self.assertRaises(ValueError,i2c_write, 1, 0, [256], 1)
        self.assertRaises(ValueError,i2c_write, 1, 0, [0], 256)
        self.assertRaises(ValueError,i2c_write, 2, 0, [0], -1)
        # Test fail coniditon for len(tx_bytes) = 255
        testlist = [1] * 256
        self.assertRaises(ValueError,i2c_write, 1, 0, testlist, 256)
        # Tests fail conditions when there is not a registered write, (reg=None)
        self.assertRaises(ValueError,i2c_write, 2, 0, [0])
        self.assertRaises(ValueError,i2c_write, 1, 256, [0])
        self.assertRaises(ValueError,i2c_write, 1, -1, [0])
        self.assertRaises(ValueError,i2c_write, 1, 0, [256])

        # Tests fail condition for can_util.next_message
        mock_next_message.return_value.data = [0,1]
        self.assertRaises(Exception,i2c_write, 0,0,[0],1)
        self.assertRaises(Exception,i2c_write, 1,255,[255,255],255)

if __name__ == '__main__':
    unittest.main()
