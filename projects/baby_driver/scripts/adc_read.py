import can_util
from gpio_port import GpioPort
from message_defs import BABYDRIVER_CAN_MESSAGE_ID, BABYDRIVER_DEVICE_ID

# Port number can be input as either an int or str
def adc_read(port, pin, raw):

    # If port is input as a str, convert to int
    if isinstance(port, str): 
        port = getattr(GpioPort, port.capitalize())

    # Data as a list of tuples
    data = [
        ("id", 4),
        ("port", port),
        ("pin", pin),
        ("raw", raw),
    ]

    # Pack data into a list of bytes
    data = can_util.can_pack(data)
    
    # Send CAN message 
    can_util.send_message(BABYDRIVER_CAN_MESSAGE_ID, data)

    # Wait to receive a CAN message with data
    data_mssg = can_util.next_message()

    # Check if message is actually an adc read
    if data_mssg.data[0] != BABYDRIVER_CAN_MESSAGE_ID:
        raise Exception("ERROR: did not receive adc read data") 

    # Extract the low and high bytes of the ADC conversion
    result_low = data_mssg.data[1]
    result_high = data_mssg.data[2]

    # Wait to receive a CAN status message indicating the end of transaction
    status_mssg = can_util.next_message()

    # Check if message status is not ok (nonzero)
    if status_mssg.data[1] != 0:
        raise Exception("ERROR: received STATUS_CODE {}".format(status_mssg.data[1]))

    # return ADC conversion values 
    return (result_high << 8) | result_low