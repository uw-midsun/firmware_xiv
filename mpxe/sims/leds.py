from mpxe.protogen import stores_pb2

from mpxe.sims import sim

class Leds(sim.Sim):
    def __init__(self):
        super(Leds, self).__init__()
        self.displays['leds'] = [{
            'type': sim.DisplayType.BOOLEAN,
            'value': False,
        }] * 4 # four leds

    def handle_update(self, pm, proj):
        states = proj.stores[(stores_pb2.MxStoreType.GPIO, 0)].state
        self.displays['leds'][0]['value'] = states[16+5]
        self.displays['leds'][0]['value'] = states[16+4]
        self.displays['leds'][0]['value'] = states[16+3]
        self.displays['leds'][0]['value'] = states[0+15]
        print('LEDs:', [int(led['value']) for led in self.displays['leds']])

    def handle_log(self, pm, proj, log):
        print('[{}]'.format(proj.name), log)
