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
    
    unfiltered_msg = can_util.Message.from_msg(msg = can_message)
    print(f"{unfiltered_msg.data} and {unfiltered_msg.message_id}")
    # if unfiltered_msg.message_id == message_defs.BABYDRIVER_CAN_MESSAGE_ID:
    #     if unfiltered_msg.data[0] == message_defs.BabydriverMessageId.GPIO_IT_INTERRUPT:
    #         data = unfiltered_msg.data[:]
    #         port = data[1]
    #         pin = data[2]
    #         edge = data[3]
    #         # Calling function from callback_dict based on (port, pin)
    #         try:
    #             callback_dict[(port, pin)]((port, pin, edge))
    #         except TypeError:
    #             print("Callback function parameters are of incorrect format")


def default_callback(info):
    port, pin, edge = info
    print(f"Interrupt on P{port}{pin}: {edge}")


# Getting bus and setting up notifier to listen for all CAN messages
def init_bus_gpio_it():
    bus_gpio_interrupt = can_util.get_bus()
    notifier = can.Notifier(bus_gpio_interrupt, [callback_listener])


def register_gpio_interrupt(port, pin, edge = InterruptEdge.RISING, callback = None):
    """
    Registers a gpio interrupt on a pin

    Args:
        port: port of the GPIO pin to register an interrupt in
        pin: pin number of the GPIO pin to register an interrupt in
        edge: callback function is called during when this interrupt edge occurs
        callback: if callback is None, a default callback function will be called that
                  prints "Interrupt on P<port><pin>: <edge>" in this format.
                  The callback function should follow this format: 
                  function_name(info = (port, pin, edge)) where the only parameter is info, 
                  a named tuple which holds port, pin and edge.
    Raises: 
    Value error: if the parameters passed into register_gpio_interrupt are incorrect
    Attribute error: if the port parameter is invalid (refer gpio_port.py for acceptable port parameters)
    Exception: if a non-zero status code is received when attempting to register an interrupt
    Type error: if the callback function (called when interrupt occurs) is of incorrect format

    """
    
    if isinstance(port, str):
        port = getattr(GpioPort, port.capitalize())

    if port < 0 or port >= GpioPort.NUM_GPIO_PORTS:
        raise ValueError("invalid GPIO port")

    if pin < 0 or pin >= NUM_PINS_PER_PORT:
        raise ValueError("invalid GPIO pin number")

    if (edge < 0 or edge >= InterruptEdge.NUM_INTERRUPT_EDGES):
        raise ValueError("invalid interrupt edge")

    if (callable(callback)) is False:
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
        raise ValueError("invalid GPIO port")

    if pin < 0 or pin >= NUM_PINS_PER_PORT:
        raise ValueError("invalid GPIO pin number")

    msg_data_unregister_gpio_interrupt = [(port, 1), (pin, 1)]

    can_util.send_message(
        babydriver_id = message_defs.BabydriverMessageId.GPIO_IT_UNREGISTER_COMMAND,
        data = can_util.can_pack(msg_data_unregister_gpio_interrupt)
    )

    status_message = can_util.next_message(babydriver_id=message_defs.BabydriverMessageId.STATUS)
    received_status = status_message.data[1]   

    # Check if status is invalid (0 refers to STATUS_CODE_OK)
    if received_status != 0:
        raise Exception("received a nonzero STATUS_CODE: {}".format(received_status))  

    # Clearing callback related to the interrupt that was unregistered
    try:
        del callback_dict[(port, pin)]
    except KeyError:
        print("no interrupt registered on given port and pin")


    


