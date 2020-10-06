from protogen import stores_pb2

class ControllerBoardBlinkingLeds:
    def handle_stores(self, pm, stores):
        gpio = stores[(stores_pb2.MxStoreType.GPIO, 0)]
        port_a = [int(gpio.state[i]) for i in range(len(gpio.state)) if i < 16]
        led_states = [port_a[9], port_a[10], port_a[11], port_a[15]]
        print('LEDs:', led_states)
