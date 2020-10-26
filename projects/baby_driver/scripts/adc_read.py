from gpio_port import GpioPort
import can_util

# Input port as char, pin as int, and raw as bool
def adc_read(port, pin, raw):
    # Convert the port label to its corresponding number
    port_num = getattr(GpioPort, port)
    data = {
        "PORT" : port_num,
        "PIN" : pin,
        "RAW" : raw,
    } 
    # Send a CAN message 
    can_util.send_message(4, data)

    # Wait to receive a CAN message  
    mssg = can_util.next_message()

    return mssg
