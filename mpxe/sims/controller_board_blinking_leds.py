from mpxe.protogen import stores_pb2

from mpxe.sims import sim

class Leds(sim.Sim):
    def handle_update(self, pm, proj):
        states = [int(i) for i in proj.stores[(stores_pb2.MxStoreType.GPIO, 0)].state]
        # use addition to indicate port - 16 = GPIO_PORT_B, 0 = GPIO_PORT_A
        leds = [states[16+5], states[16+4], states[16+3], states[0+15]]
        print('LEDs:', leds)

    def handle_log(self, pm, proj, log):
        print('[{}]'.format(proj.name), log)
