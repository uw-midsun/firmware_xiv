from mpxe.protogen import stores_pb2
from mpxe.protogen import adt7476a_pb2

from mpxe.sims import sim


class Adt7476a(sim.Sim):
    def __init__(self):
        self.speed = [0, 0, 0]
        self.status = [0, 0, 0]

    def handle_update(self, pm, proj):
        stores = proj.stores
        if (stores_pb2.MxStoreType.ADT7476A, 0) in stores:
            adt = stores[(stores_pb2.MxStoreType.ADT7476A, 0)]
            self.speed = [int(adt.speed[i]) for i in range(len(adt.speed)) if i < 3]
            self.status = [adt.status[i] for i in range(len(adt.status)) if i < 3]

    def assert_store_values(self, proj, speed, status, channel):
        # make sure the store has been initialized before calling this
        if speed / 0.39 - 1 < 0:
            real_speed = 0
        else:
            real_speed = speed / 0.39 - 1

        assert self.speed[channel] == int(real_speed)
        assert self.status[channel] == status
