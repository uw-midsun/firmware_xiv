from mpxe.protogen import stores_pb2

from mpxe.sims import sim

class Adt7476a(sim.Sim):
    def __init__(self):
        self.speed = None
        self.status = None

    def handle_update(self, pm, proj):
        stores  = proj.stores
        if (stores_pb2.MxStoreType.ADT7476A, 0) in stores:
            adt         = stores[(stores_pb2.MxStoreType.ADT7476A, 0)]
            self.speed  = [int(adt.speed[i]) for i in range(len(adt.speed)) if i < 3]
            self.status = [adt.status[i] for i in range(len(adt.status)) if i < 3]

    def assert_store_values(self, proj, speed, status, channel):
        # make sure the store has been initialized before calling this

        real_speed = speed / 0.39
        # real_speed is what's actually passed
        assert self.speed[channel]  == int(real_speed)
        assert self.status[channel] == status
