from can_util import send_message, next_message
from message_defs import BabydriverMessageId

class GpioPort:
    A = 0
    B = 1
    C = 2
    D = 3
    E = 4
    F = 5

def gpio_set(port, pin, state):
    if isinstance(port, str):
        # nasty hack
        port = getattr(GpioPort, port.upper())

    data = [port, pin, state]
    send_message(BabydriverMessageId.GPIO_SET, data)

    status = next_message(BabydriverMessageId.STATUS).data[1]
    if status:
        raise Exception("gpio_set failed with status code {}; see status.h".format(status))
    
    # print('OK')
