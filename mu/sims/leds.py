from mu.harness.board_sim import BoardSim, GPIO_KEY

# pylint: disable=useless-super-delegation
class Leds(BoardSim):
    def __init__(self, pm, proj_name='leds'):
        super().__init__(pm, proj_name)

    def handle_store(self, store, key):
        states = [int(i) for i in self.stores[GPIO_KEY].state]
        # use addition to indicate port - 16 = GPIO_PORT_B, 0 = GPIO_PORT_A
        leds = [states[16 + 5], states[16 + 4], states[16 + 3], states[0 + 15]]
        self.pm.logger.log(self.__class__.__name__, str(leds))
