import stores_pb2
import gpio_pb2

# Temporary demonstration handler
def handle_gpio(msg):
    gpio = gpio_pb2.MxGpioStore()
    gpio.ParseFromString(msg)
    port_a = [int(gpio.state[i]) for i in range(len(gpio.state)) if i < 16]
    print('A:', port_a)

def handle_store(msg: bytes):
    store_info = stores_pb2.MxStoreInfo()
    store_info.ParseFromString(msg)
    func_map[store_info.type](store_info.msg)

# This will eventually get big and ugly. Would be
# good to dynamically generate, or be passed in by
# argument from library user.
func_map = {
    stores_pb2.EnumStoreType.GPIO: handle_gpio,
}
