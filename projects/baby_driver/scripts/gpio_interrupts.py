"""Python implementation of register_gpio_interrupt function"""

import can
import can_util
from gpio_port import GpioPort
import message_defs

NUM_PINS_PER_PORT = 16


class InterruptEdge:
    RISING = 0
    FALLING = 1
    RISING_AND_FALLING = 2
    NUM_INTERRUPT_EDGES = 3

# Callbacks that user defines when registering interrupt are stored here  
callback_dict = {
    #("port", "pin"): callback,
}

bus_gpio_interrupt = None

def callback_listener(can_message):
    unfiltered_msg = can_util.Message.from_msg(can_message)
    # print(f"{unfiltered_msg.message_id}, {unfiltered_msg.data[0]}, {unfiltered_msg.data[1]}, {unfiltered_msg.data[1]}")
    
    if (unfiltered_msg.message_id == message_defs.BABYDRIVER_CAN_MESSAGE_ID and 
        unfiltered_msg.data[0] == message_defs.BabydriverMessageId.GPIO_IT_INTERRUPT):

        data = unfiltered_msg.data[:]
        port = data[1]
        pin = data[2]
        edge = data[3]
        # Calling function from callback_dict based on (port, pin)
        try:
            # ret is only used for testing purposes
            ret = callback_dict[(port, pin)]((port, pin, edge))
        except:
            raise("Callback function parameters are of incorrect format")
    
    return ret


def default_callback(info):
    port, pin, edge = info
    port = chr(port + ord('A'))
    print(f"Interrupt on P{port}{pin}: {edge}")


# Getting bus and setting up notifier to listen for all CAN messages
def init_bus_gpio_it():
    bus_gpio_interrupt = can_util.get_bus()
    notifier = can_util.get_bus_notifier()
    notifier.add_listener(callback_listener)


def register_gpio_interrupt(port, pin, edge = InterruptEdge.RISING, callback = None):
    """
    Registers a gpio interrupt on a pin

    Args:
        port: port of the GPIO pin to register an interrupt in
        pin: pin number of the GPIO pin to register an interrupt in
        edge: callback function is called when this interrupt edge occurs. Can be entered as a string 
              or a number (RISING (0), FALLING (1) or RISING_AND_FALLING (2))
        callback: if callback is None, a default callback function will be called that
                  prints "Interrupt on P<port><pin>: <edge>" in this format.
                  The callback function should follow this format: 
                  function_name(<info>) where the only parameter is info, 
                  a named tuple which holds port (info[0]),pin (info[1]) and edge (info[2]) 
                  of the GPIO interrupt that occured.
    Raises: 
        Value error: if the parameters passed into register_gpio_interrupt are incorrect
        Attribute error: if the port parameter or interrupt edge is invalid (refer gpio_port.py for acceptable 
                         port parameters and InterruptEdge for appropriate edge parameters)
        Exception: if a non-zero status code is received when attempting to register an interrupt
        Type error: if the callback function (called when interrupt occurs) is of incorrect format

    Note: There is a hard STM32 limit that only one GPIO interrupt can be registered at a time 
    per pin number. For example, PA2 and PB2 would share the same GPIO pin. 
    """
    
    if isinstance(port, str):
        port = getattr(GpioPort, port.capitalize())

    if port < 0 or port >= GpioPort.NUM_GPIO_PORTS:
        raise ValueError("invalid GPIO port (Range: 'A' - 'F')")

    if pin < 0 or pin >= NUM_PINS_PER_PORT:
        raise ValueError("Invalid GPIO pin number (Range: 0 - 15)")

    if isinstance(edge, str):
        if hasattr(InterruptEdge, edge.upper()) is False:
            raise AttributeError("Enter 'RISING', 'FALLING' or 'RISING_AND_FALLING' for interrupt edge")
        edge = getattr(InterruptEdge, edge.upper())
    
    if (edge < 0 or edge >= InterruptEdge.NUM_INTERRUPT_EDGES):
        raise ValueError("invalid interrupt edge (enter 0 (Rising), 1 (Falling) or 2 (Rising_and_falling")

    if callback != None and (callable(callback)) == False:
        raise ValueError("invalid callback function")

    msg_data_register_gpio_interrupt = [(port, 1), (pin, 1), (edge, 1)]

    can_util.send_message(
        babydriver_id = message_defs.BabydriverMessageId.GPIO_IT_REGISTER_COMMAND,
        data = can_util.can_pack(msg_data_register_gpio_interrupt)
    )

    status_message = can_util.next_message(babydriver_id=message_defs.BabydriverMessageId.STATUS)
    received_status = status_message.data[1]

    # Check if status is invalid (0 refers to STATUS_CODE_OK)
    if received_status != 0:
        raise Exception("received a nonzero STATUS_CODE: {}".format(received_status))

    # Adding callback to dictionary to be called upon when interrupt occurs
    if callback is None:   
        callback_dict[(port, pin)] = default_callback
    else:
        callback_dict[(port, pin)] = callback



def unregister_gpio_interrupt(port, pin):
    """
    Unregisters a gpio interrupt on a pin

    Args:
        port: Port of the GPIO pin to register an interrupt in
        pin: Pin number of the GPIO pin to register an interrupt in
    
    Raises: 
        Value error: if the parameters passed into register_gpio_interrupt are incorrect
        Attribute error: if the port parameter is invalid (refer gpio_port.py for acceptable port parameters)
        Exception: if a non-zero status code is received when attempting to uregister an interrupt
        Key error: if no interrupt is registered in the given port and pin
    """   

    if isinstance(port, str):
        port = getattr(GpioPort, port.capitalize())

    if port < 0 or port >= GpioPort.NUM_GPIO_PORTS:
        raise ValueError("invalid GPIO port (Range: 'A' - 'F')")

    if pin < 0 or pin >= NUM_PINS_PER_PORT:
        raise ValueError("Invalid GPIO pin number (Range: 0 - 15)")

    msg_data_unregister_gpio_interrupt = [(port, 1), (pin, 1)]

    can_util.send_message(
        babydriver_id = message_defs.BabydriverMessageId.GPIO_IT_UNREGISTER_COMMAND,
        data = can_util.can_pack(msg_data_unregister_gpio_interrupt)
    )

    status_message = can_util.next_message(babydriver_id=message_defs.BabydriverMessageId.STATUS)
    received_status = status_message.data[1]   

    # Check if status is invalid (0 refers to STATUS_CODE_OK)
    if received_status != 0:
        raise Exception("Received a nonzero STATUS_CODE: {}".format(received_status))  

    # Clearing callback related to the interrupt that was unregistered
    if (port, pin) not in callback_dict:
        raise KeyError(f"No interrupt registered on given port and pin {callback_dict}") 
  
    del callback_dict[(port, pin)]
 




    


