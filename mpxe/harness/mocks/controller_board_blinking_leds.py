from protogen import stores_pb2

class ControllerBoardBlinkingLeds:
    def handle_stores(self, pm, stores):
        gpio = stores[(stores_pb2.MxStoreType.GPIO, 0)]
        port_a = [int(gpio.state[i]) for i in range(len(gpio.state)) if i < 16]
        print('A:', port_a)