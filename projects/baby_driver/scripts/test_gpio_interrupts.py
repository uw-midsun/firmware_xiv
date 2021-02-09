"""This Module Tests methods in gpio_interrupts.py"""
import unittest
from unittest.mock import patch

from gpio_interrupts import (register_gpio_interrupt, unregister_gpio_interrupt, 
                            NUM_PINS_PER_PORT, NUM_INTERRUPT_EDGES, callback_dict)
from gpio_port import GpioPort
from message_defs import BABYDRIVER_DEVICE_ID, BABYDRIVER_CAN_MESSAGE_ID
    
# pylint: disable=unused-argument
class test_register_gpio_interrupt(unittest.TestCase):
    """Test register_gpio_interrupt function"""

    @patch('can_util.send_message')
    @patch('can_util.next_message')
    # pylint: disable=no-self-use
    def test_register_gpio_it_parameters(self, mock_next_message, mock_send_message):
        """Tests parameters passed into register_gpio_interrupt"""
        mock_next_message.return_value.data = [0, 0]

        # Tests minimum values for port, pin, and interrupt edge 
        register_gpio_interrupt('A', 4, 1, None) 
        register_gpio_interrupt('B', 0, 2, None) 
        register_gpio_interrupt(GpioPort.A, 5, 0, None) 
        register_gpio_interrupt(GpioPort.A, 0, 0, None) 

        # Tests maximum values for port, pin, and interrupt edge
        register_gpio_interrupt('F', 5, 1, None) 
        register_gpio_interrupt(GpioPort.D, 15, 1, None) 
        register_gpio_interrupt(GpioPort.E, 5, 2, None) 
        register_gpio_interrupt('F', 15, 2, None) 

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
        register_gpio_interrupt(0, 8, 2, None)
        self.assertEqual(14, self.babydriver_id)
        self.assertEqual([0, 8, 2], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)

        register_gpio_interrupt(2, 0, 2, None)
        self.assertEqual(14, self.babydriver_id)
        self.assertEqual([2, 0, 2], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)

        register_gpio_interrupt(2, 8, 0, None)
        self.assertEqual(14, self.babydriver_id)
        self.assertEqual([2, 8, 0], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)

        # Tests max parameters for can_util.send_message (port, pin, edge)
        register_gpio_interrupt(GpioPort.F, 8, 1, None)
        self.assertEqual(14, self.babydriver_id)
        self.assertEqual([GpioPort.F, 8, 1], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)

        register_gpio_interrupt(2, GpioPort.NUM_PINS_PER_PORT - 1, 1, None)
        self.assertEqual(14, self.babydriver_id)
        self.assertEqual([2, GpioPort.NUM_PINS_PER_PORT - 1, 1], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)

        register_gpio_interrupt(2, 8, NUM_INTERRUPT_EDGES - 1, None)
        self.assertEqual(14, self.babydriver_id)
        self.assertEqual([2, 8, NUM_INTERRUPT_EDGES - 1], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)    

    @patch('can_util.send_message')
    @patch('can_util.next_message')
    def test_fail_conditions(self, mock_next_message, mock_send_message):
        """Tests fail conditions"""

        # Tests fail condition from register_gpio_interrupt parameter input
        self.assertRaises(AttributeError, register_gpio_interrupt, 'G', 0, 0, None)
        self.assertRaises(ValueError, register_gpio_interrupt, GpioPort.NUM_GPIO_PORTS, 4, 1, None)
        self.assertRaises(AttributeError, register_gpio_interrupt, -1, 9, 2, None)

        self.assertRaises(ValueError, register_gpio_interrupt, GpioPort.B, NUM_PINS_PER_PORT, 0, None)
        self.assertRaises(ValueError, register_gpio_interrupt, 'e', -1, 0, None)

        self.assertRaises(ValueError, register_gpio_interrupt, GpioPort.D, 0, -1, None)
        self.assertRaises(ValueError, register_gpio_interrupt, 'A', 15, NUM_INTERRUPT_EDGES, None)

        not_callable = 0
        self.assertRaises(ValueError, register_gpio_interrupt, 'A', 0, 0, not_callable)

        # Tests failing status code from can_util.next_message
        mock_next_message.return_value.data = [0, 1]
        self.assertRaises(Exception, register_gpio_interrupt, 'A', 0, 0, None)
        self.assertRaises(Exception, register_gpio_interrupt, 'F', 15, 2, None)






# pylint: disable=unused-argument
class test_unregister_gpio_interrupt(unittest.TestCase):
    """Test unregister_gpio_interrupt function"""

    @patch('can_util.send_message')
    @patch('can_util.next_message')
    # pylint: disable=no-self-use
    def test_unregister_gpio_it_parameters(self, mock_next_message, mock_send_message):
        """Tests parameters passed into unregister_gpio_interrupt"""
        mock_next_message.return_value.data = [0, 0]

        # Tests minimum values for port, pin, and interrupt edge 
        unregister_gpio_interrupt('A', 4, 1, None) 
        unregister_gpio_interrupt('B', 0, 2, None) 
        unregister_gpio_interrupt(GpioPort.A, 5, 0, None) 
        unregister_gpio_interrupt(GpioPort.A, 0, 0, None) 

        # Tests maximum values for port, pin, and interrupt edge
        unregister_gpio_interrupt('F', 5, 1, None) 
        unregister_gpio_interrupt(GpioPort.D, 15, 1, None) 
        unregister_gpio_interrupt(GpioPort.E, 5, 2, None) 
        unregister_gpio_interrupt('F', 15, 2, None) 

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
        unregister_gpio_interrupt(0, 8, 2, None)
        self.assertEqual(14, self.babydriver_id)
        self.assertEqual([0, 8, 2], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)

        unregister_gpio_interrupt(2, 0, 2, None)
        self.assertEqual(14, self.babydriver_id)
        self.assertEqual([2, 0, 2], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)

        unregister_gpio_interrupt(2, 8, 0, None)
        self.assertEqual(14, self.babydriver_id)
        self.assertEqual([2, 8, 0], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)

        # Tests max parameters for can_util.send_message (port, pin, edge)
        unregister_gpio_interrupt(GpioPort.F, 8, 1, None)
        self.assertEqual(14, self.babydriver_id)
        self.assertEqual([GpioPort.F, 8, 1], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)

        unregister_gpio_interrupt(2, GpioPort.NUM_PINS_PER_PORT - 1, 1, None)
        self.assertEqual(14, self.babydriver_id)
        self.assertEqual([2, GpioPort.NUM_PINS_PER_PORT - 1, 1], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)

        unregister_gpio_interrupt(2, 8, NUM_INTERRUPT_EDGES - 1, None)
        self.assertEqual(14, self.babydriver_id)
        self.assertEqual([2, 8, NUM_INTERRUPT_EDGES - 1], self.data)
        self.assertEqual(None, self.channel)
        self.assertEqual(BABYDRIVER_CAN_MESSAGE_ID, self.msg_id)
        self.assertEqual(BABYDRIVER_DEVICE_ID, self.device_id)    

    @patch('can_util.send_message')
    @patch('can_util.next_message')
    def test_fail_conditions(self, mock_next_message, mock_send_message):
        """Tests fail conditions"""

        # Tests fail condition from register_gpio_interrupt parameter input
        self.assertRaises(AttributeError, unregister_gpio_interrupt, 'G', 0, 0, None)
        self.assertRaises(ValueError, unregister_gpio_interrupt, GpioPort.NUM_GPIO_PORTS, 4, 1, None)
        self.assertRaises(AttributeError, unregister_gpio_interrupt, -1, 9, 2, None)

        self.assertRaises(ValueError, unregister_gpio_interrupt, GpioPort.B, NUM_PINS_PER_PORT, 0, None)
        self.assertRaises(ValueError, unregister_gpio_interrupt, 'e', -1, 0, None)

        self.assertRaises(ValueError, unregister_gpio_interrupt, GpioPort.D, 0, -1, None)
        self.assertRaises(ValueError, register_gpio_interrupt, 'A', 15, NUM_INTERRUPT_EDGES, None)

        not_callable = 0
        self.assertRaises(ValueError, unregister_gpio_interrupt, 'A', 0, 0, not_callable)

        # Tests failing status code from can_util.next_message
        mock_next_message.return_value.data = [0, 1]
        self.assertRaises(Exception, unregister_gpio_interrupt, 'A', 0, 0, None)
        self.assertRaises(Exception, unregister_gpio_interrupt, 'F', 15, 2, None)

        # Tests fail condition for no registered interrupt on a given pin and port
        mock_next_message.return_value.data = [0, 0]
        register_gpio_interrupt(0, 0, 0, None)
        unregister_gpio_interrupt(0, 0, 0, None)
        self.assertRaises(KeyError, unregister_gpio_interrupt, 0, 0, 0, None)

if __name__ == "__main__":
    unittest.main()





        
