
import unittest
from unittest.mock import patch

from i2c_write import i2c_write
from message_defs import BABYDRIVER_DEVICE_ID, BABYDRIVER_CAN_MESSAGE_ID

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

    def test_send_message(self, mock_next_message, mock_send_message):
        """Tests accuracy of parameters passed into can_util.send_message"""

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
        i2c_write(0, 10, [0], 1)
        self.assertEqual(9, self.babydriver_id)
        self.assertEqual([0], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)

        i2c_write(1, 0, [0,0,0], 1)
        self.assertEqual(9, self.babydriver_id)
        self.assertEqual([0,0,0], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)

        i2c_write(1, 10, [7, 5, 12], 0)
        self.assertEqual(9, self.babydriver_id)
        self.assertEqual([7, 5, 12], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)

        i2c_write(1, 10, [7, 5, 12, 14, 15, 18, 9], 1)
        self.assertEqual(9, self.babydriver_id)
        self.assertEqual([7, 5, 12, 14, 15, 18, 9], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)


        # Tests high parameters for can_util.send_message
        i2c_write(1, 10, [7, 5, 12, 7, 5, 12, 8, 1, 2, 3, 4, 5, 6, 7], 1)
        self.assertEqual(9, self.babydriver_id)
        self.assertEqual([1, 2, 3, 4, 5, 6, 7], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)

        i2c_write(1, 255, [7, 5, 12, 7, 5, 12, 7, 255, 255, 255, 255, 255, 255, 255], 1)
        self.assertEqual(9, self.babydriver_id)
        self.assertEqual([255, 255, 255, 255, 255, 255, 255], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)

        i2c_write(1, 10, [255,255,255], 1)
        self.assertEqual(9, self.babydriver_id)
        self.assertEqual([255, 255, 255], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)

        i2c_write(1, 10, [7, 5, 12], 255)
        self.assertEqual(9, self.babydriver_id)
        self.assertEqual([7, 5, 12], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)

    @patch('can_util.send_message')
    @patch('can_util.next_message')
    def test_fail_conditions(self, mock_next_messaage, mock_send_message):
        """Tests fail conditions"""

        # Tests fail condition for i2c_write parameter input
        self.assertRaises(ValueError,i2c_write, 2, 0, [0], 1)
        self.assertRaises(ValueError,i2c_write, 1, 266, [0], 1)
        self.assertRaises(ValueError,i2c_write, 1, -1, [0], 1)
        self.assertRaises(ValueError,i2c_write, 1, 0, [266], 1)
        self.assertRaises(ValueError,i2c_write, 1, 0, [0], 266)
        self.assertRaises(ValueError,i2c_write, 2, 0, [0], -1)

        # Tests fail condition for can_util.next_message
        mock_next_messaage.return_value.data = [0,1]
        self.assertRaises(Exception,i2c_write, 0,0,[0],1)
        self.assertRaises(Exception,i2c_write, 1,255,[255,255],255)

if __name__ == '__main__':
    unittest.main()















