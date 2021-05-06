from mu.harness.sub_sim import SubSim
from mu.protogen import stores_pb2

ADT7476A_KEY = (stores_pb2.MuStoreType.ADT7476A, 0)


class Adt7476a(SubSim):
    def assert_values(self, speed, status, channel):
        # make sure the store has been initialized before calling this
        if speed / 0.39 - 1 < 0:
            real_speed = 0
        else:
            real_speed = speed / 0.39 - 1

        stores = self.parent.stores
        if ADT7476A_KEY in stores:
            adt = stores[ADT7476A_KEY]
            speeds = [int(adt.speed[i]) for i in range(len(adt.speed)) if i < 3]
            statuses = [adt.status[i] for i in range(len(adt.status)) if i < 3]

            assert speeds[channel] == int(real_speed)
            assert statuses[channel] == status
