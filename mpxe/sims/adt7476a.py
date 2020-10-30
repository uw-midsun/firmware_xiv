from mpxe.protogen import stores_pb2
from mpxe.protogen import adt7476a_pb2

from mpxe.sims import sim

class Adt7476a(sim.Sim):
    def handle_update(self, pm, proj):
        stores  = proj.stores
        # for some reason there's a GPIO store initializes before
        # a ADT7476a store is initialized
        # that's why I need to if to check first
        # print(stores)
        if (stores_pb2.MxStoreType.ADT7476A, 0) in stores:
            adt         = stores[(stores_pb2.MxStoreType.ADT7476A, 0)]
            self.speed  = [int(adt.speed[i]) for i in range(len(adt.speed)) if i < 3]
            self.status = [adt.status[i] for i in range(len(adt.status)) if i < 3]

    def handle_log(self, pm, proj, log):
        print('[{}]'.format(proj.name), log)

    def assert_store_value(self, proj, speed, status, channel):
        # you need to make sure the store has been initialized before calling this
        real_speed = speed / 0.39
        assert(self.speed[channel]  == int(real_speed))   # because real_speed is what's actually passed
        assert(self.status[channel] == status)
