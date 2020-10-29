from mpxe.protogen import stores_pb2
from mpxe.protogen import adt7476a_pb2

from mpxe.sims import sim

class Adt7476a(sim.Sim):
    def handle_update(self, pm, proj):
        # stores = proj.stores
        # gpio = stores[(stores_pb2.MxStoreType.GPIO, 0)]
        # port_a = [int(gpio.state[i]) for i in range(len(gpio.state)) if i < 16]
        # led_states = [port_a[9], port_a[10], port_a[11], port_a[15]]
        print("updating")

    def handle_log(self, pm, proj, log):
        print('[{}]'.format(proj.name), log)
