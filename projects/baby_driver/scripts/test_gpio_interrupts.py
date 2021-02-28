"""This Module Tests methods in gpio_interrupts.py"""
import unittest
from unittest.mock import patch

from gpio_interrupts import (register_gpio_interrupt, unregister_gpio_interrupt,
                            NUM_PINS_PER_PORT, InterruptEdge, callback_dict,
                            callback_listener)
from gpio_port import GpioPort
import can_util
from message_defs import BABYDRIVER_DEVICE_ID, BABYDRIVER_CAN_MESSAGE_ID, BabydriverMessageId

# pylint: disable=unused-argument
class TestRegisterGpioInterrupt(unittest.TestCase):
    """Test register_gpio_interrupt function"""

    @patch('can_util.send_message')
    @patch('can_util.next_message')
    # pylint: disable=no-self-use
    def test_register_gpio_it_parameters(self, mock_next_message, mock_send_message):
        """Tests parameters passed into register_gpio_interrupt"""
        mock_next_message.return_value.data = [0, 0]

        # Tests minimum values for port, pin, and interrupt edge
        register_gpio_interrupt('A', 4, 1)
        register_gpio_interrupt('B', 0, 2)
        register_gpio_interrupt(GpioPort.A, 5, 0)
        register_gpio_interrupt(GpioPort.A, 0, 'rising')

        # Tests maximum values for port, pin, and interrupt edge
        register_gpio_interrupt('F', 5, 1)
        register_gpio_interrupt(GpioPort.D, 15, 'FALLING')
        register_gpio_interrupt(GpioPort.E, 5, 2)
        register_gpio_interrupt('F', 15, 'rising_and_falling')

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
            babydriver_id = None,
            data = None,
            channel = None,
            msg_id = BABYDRIVER_CAN_MESSAGE_ID,
            device_id = BABYDRIVER_DEVICE_ID,
        ):
            self.babydriver_id = babydriver_id
            self.data = data
            self.channel = channel
            self.msg_id = msg_id
            self.device_id = device_id

        mock_send_message.side_effect = parameter_test
        mock_next_message.return_value.data = [0, 0]

        # Tests min parameters for can_util.send_message (port, pin, edge)
        register_gpio_interrupt(0, 8, 2)
        self.assertEqual(14, self.babydriver_id)
        self.assertEqual([0, 8, 2], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)

        register_gpio_interrupt(2, 0, 2)
        self.assertEqual(14, self.babydriver_id)
        self.assertEqual([2, 0, 2], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)

        register_gpio_interrupt(2, 8, 0)
        self.assertEqual(14, self.babydriver_id)
        self.assertEqual([2, 8, 0], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)

        # Tests max parameters for can_util.send_message (port, pin, edge)
        register_gpio_interrupt(GpioPort.F, 8, 1)
        self.assertEqual(14, self.babydriver_id)
        self.assertEqual([GpioPort.F, 8, 1], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)

        register_gpio_interrupt(2, NUM_PINS_PER_PORT - 1, 1)
        self.assertEqual(14, self.babydriver_id)
        self.assertEqual([2, NUM_PINS_PER_PORT - 1, 1], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)

        register_gpio_interrupt(2, 8, InterruptEdge.NUM_INTERRUPT_EDGES - 1)
        self.assertEqual(14, self.babydriver_id)
        self.assertEqual([2, 8, InterruptEdge.NUM_INTERRUPT_EDGES - 1], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)

    @patch('can_util.send_message')
    @patch('can_util.next_message')
    def test_fail_conditions(self, mock_next_message, mock_send_message):
        """Tests fail conditions"""

        # Tests fail condition from register_gpio_interrupt parameter input
        self.assertRaises(AttributeError, register_gpio_interrupt, 'G', 0, 0)
        self.assertRaises(ValueError, register_gpio_interrupt, GpioPort.NUM_GPIO_PORTS, 4, 1)
        self.assertRaises(ValueError, register_gpio_interrupt, -1, 9, 2)

        self.assertRaises(ValueError, register_gpio_interrupt, GpioPort.B, NUM_PINS_PER_PORT, 0)
        self.assertRaises(ValueError, register_gpio_interrupt, 'e', -1, 0)

        self.assertRaises(ValueError, register_gpio_interrupt, GpioPort.D, 0, -1)
        self.assertRaises(ValueError, register_gpio_interrupt, 'A', 15,
                          InterruptEdge.NUM_INTERRUPT_EDGES)
        self.assertRaises(AttributeError, register_gpio_interrupt, 'A', 15, 'no_edge')

        self.assertRaises(ValueError, register_gpio_interrupt, 'A', 0, 0, 5)
        self.assertRaises(ValueError, register_gpio_interrupt, 'A', 0, 0, "not_callable")

        # Tests failing status code from can_util.next_message
        mock_next_message.return_value.data = [0, 1]
        self.assertRaises(Exception, register_gpio_interrupt, 'A', 0, 0)
        self.assertRaises(Exception, register_gpio_interrupt, 'F', 15, 2)


# pylint: disable=unused-argument
class TestCallbackListener(unittest.TestCase):
    """Test the callback_listener function"""

    @patch('can_util.Message.from_msg')
    @patch('can_util.send_message')
    @patch('can_util.next_message')
    # pylint: disable=no-self-use
    def test_params_and_fail_conditions(self, mock_next_message, mock_send_message,
                                        mock_convert_can_msg):
        """
        Tests callback_listener by checking if it can trigger default/user-defined callbacks
        upon receving can messages and tests fail conditions
        """

        # Stores parameters passed into can_util.send_message
        # pylint: disable=attribute-defined-outside-init
        self.babydriver_id = None
        self.data = None
        self.channel = None
        self.msg_id = None
        self.device_id = None

        def parameter_test(
            babydriver_id = None,
            data = None,
            channel = None,
            msg_id = BABYDRIVER_CAN_MESSAGE_ID,
            device_id = BABYDRIVER_DEVICE_ID,
        ):
            self.babydriver_id = babydriver_id
            self.data = data
            self.channel = channel
            self.msg_id = msg_id
            self.device_id = device_id

        mock_send_message.side_effect = parameter_test
        mock_next_message.return_value.data = [0, 0]
        self.test_user_callback_output = None

        def test_user_callback(info):
            self.test_user_callback_output = ("Test callback, port:{}, pin:{}, "
                                              "edge:{}".format(info.port, info.pin, info.edge))

        def incorrect_test_user_callback(port, pin, edge):
            self.test_user_callback_output =  ("Incorrect Test callback, {}, {},"
                                               " {}".format(port, pin, edge))

        def test_msg_converter(can_message = None):
            ret_msg = can_util.Message(message_id = can_message[0], data = can_message[1][:],
                                       device_id = BABYDRIVER_DEVICE_ID)
            return ret_msg

        mock_convert_can_msg.side_effect = test_msg_converter

        callback_dict.clear()
        register_gpio_interrupt(0,0,0,test_user_callback)
        register_gpio_interrupt(5,3,0,test_user_callback)
        register_gpio_interrupt(2,15,0,test_user_callback)
        register_gpio_interrupt(5,15,0,test_user_callback)

        it_msg_id = BabydriverMessageId.GPIO_IT_INTERRUPT

        # The 2 element array represents a mock of the can message received from the firmware side
        # Testing function for keys stored in callback_dict
        data = [it_msg_id, 0, 0, 1]
        callback_listener([BABYDRIVER_CAN_MESSAGE_ID, data])
        self.assertEqual(self.test_user_callback_output,"Test callback, port:0, pin:0, edge:1")

        data = [it_msg_id, 5, 3, 0]
        callback_listener([BABYDRIVER_CAN_MESSAGE_ID, data])
        self.assertEqual(self.test_user_callback_output, "Test callback, port:5, pin:3, edge:0")

        data = [it_msg_id, 2, 15, 2]
        callback_listener([BABYDRIVER_CAN_MESSAGE_ID, data])
        self.assertEqual(self.test_user_callback_output, "Test callback, port:2, pin:15, edge:2")

        data = [it_msg_id, 5, 15, 1]
        callback_listener([BABYDRIVER_CAN_MESSAGE_ID, data])
        self.assertEqual(self.test_user_callback_output, "Test callback, port:5, pin:15, edge:1")

        #Testing fail condition
        callback_dict.clear()
        register_gpio_interrupt(4,8,0,incorrect_test_user_callback)

        data = [it_msg_id, 4, 8, 2]
        self.assertRaises(TypeError, callback_listener, [BABYDRIVER_CAN_MESSAGE_ID, data])



# pylint: disable=unused-argument
class TestUnregisterGpioInterrupt(unittest.TestCase):
    """Test unregister_gpio_interrupt function"""

    @patch('can_util.send_message')
    @patch('can_util.next_message')
    # pylint: disable=no-self-use
    def test_unregister_gpio_it_parameters(self, mock_next_message, mock_send_message):
        """Tests parameters passed into unregister_gpio_interrupt"""
        mock_next_message.return_value.data = [0, 0]

        # Clearing and reinitializing callback_dict
        callback_dict.clear()

        register_gpio_interrupt('A', 4, 0)
        register_gpio_interrupt('B', 0, 0)
        register_gpio_interrupt(3, 0, 0)
        register_gpio_interrupt(GpioPort.A, 0, 0)

        register_gpio_interrupt('F', 5, 0)
        register_gpio_interrupt(GpioPort.D, 15, 0)
        register_gpio_interrupt(5, 3, 0)
        register_gpio_interrupt('F', 15, 0)

        # Tests minimum values for port, pin, and interrupt edge
        unregister_gpio_interrupt('A', 4)
        unregister_gpio_interrupt('B', 0)
        unregister_gpio_interrupt(3, 0)
        unregister_gpio_interrupt(GpioPort.A, 0)

        # Tests maximum values for port, pin, and interrupt edge
        unregister_gpio_interrupt('F', 5)
        unregister_gpio_interrupt(GpioPort.D, 15)
        unregister_gpio_interrupt(5, 3)
        unregister_gpio_interrupt('F', 15)

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
            babydriver_id = None,
            data = None,
            channel = None,
            msg_id = BABYDRIVER_CAN_MESSAGE_ID,
            device_id = BABYDRIVER_DEVICE_ID,
        ):
            self.babydriver_id = babydriver_id
            self.data = data
            self.channel = channel
            self.msg_id = msg_id
            self.device_id = device_id

        mock_send_message.side_effect = parameter_test
        mock_next_message.return_value.data = [0, 0]

        # Tests min parameters for can_util.send_message (port, pin, edge)
        unregister_gpio_interrupt(0, 8)
        self.assertEqual(15, self.babydriver_id)
        self.assertEqual([0, 8], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)

        unregister_gpio_interrupt(2, 0)
        self.assertEqual(15, self.babydriver_id)
        self.assertEqual([2, 0], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)


        # Tests max parameters for can_util.send_message (port, pin)
        unregister_gpio_interrupt(GpioPort.F, 8)
        self.assertEqual(15, self.babydriver_id)
        self.assertEqual([GpioPort.F, 8], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)

        unregister_gpio_interrupt(2, NUM_PINS_PER_PORT - 1)
        self.assertEqual(15, self.babydriver_id)
        self.assertEqual([2, NUM_PINS_PER_PORT - 1], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)


    @patch('can_util.send_message')
    @patch('can_util.next_message')
    def test_fail_conditions(self, mock_next_message, mock_send_message):
        """Tests fail conditions"""

        # Stores parameters passed into can_util.send_message
        # pylint: disable=attribute-defined-outside-init 
        self.babydriver_id = None
        self.data = None
        self.channel = None
        self.msg_id = None
        self.device_id = None

        def parameter_test(
            babydriver_id = None,
            data = None,
            channel = None,
            msg_id = BABYDRIVER_CAN_MESSAGE_ID,
            device_id = BABYDRIVER_DEVICE_ID,
        ):
            self.babydriver_id = babydriver_id
            self.data = data
            self.channel = channel
            self.msg_id = msg_id
            self.device_id = device_id

        mock_send_message.side_effect = parameter_test
        mock_next_message.return_value.data = [0, 0]

        # Tests fail condition from register_gpio_interrupt parameter input
        self.assertRaises(AttributeError, unregister_gpio_interrupt, 'G', 0)
        self.assertRaises(ValueError, unregister_gpio_interrupt, GpioPort.NUM_GPIO_PORTS, 4)
        self.assertRaises(ValueError, unregister_gpio_interrupt, -1, 9)

        self.assertRaises(ValueError, unregister_gpio_interrupt, GpioPort.B, NUM_PINS_PER_PORT)
        self.assertRaises(ValueError, unregister_gpio_interrupt, 'e', -1)

        # Tests failing status code from can_util.next_message
        mock_next_message.return_value.data = [0, 1]
        self.assertRaises(Exception, unregister_gpio_interrupt, 'A', 0)
        self.assertRaises(Exception, unregister_gpio_interrupt, 'F', 15)

        # Tests fail condition for no registered interrupt on a given pin and port
        mock_next_message.return_value.data = [0, 0]
        register_gpio_interrupt(0, 0, 0)
        unregister_gpio_interrupt(0, 0)
        self.assertRaises(KeyError, unregister_gpio_interrupt, 0, 0)

if __name__ == "__main__":
    unittest.main()
