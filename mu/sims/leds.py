from mu.harness.board_sim import BoardSim, GPIO_KEY
from mu.protogen import stores_pb2


class Leds(BoardSim):
    def handle_store(self, store, key):
        states = [int(i) for i in self.stores[GPIO_KEY].state]
        # use addition to indicate port - 16 = GPIO_PORT_B, 0 = GPIO_PORT_A
        leds = [states[16 + 5], states[16 + 4], states[16 + 3], states[0 + 15]]
        print('LEDs:', leds)
