from mu.harness.project import StoreUpdate
from mu.harness.sub_sim import SubSim
from mu.protogen import stores_pb2
from mu.protogen import pca9539r_pb2

NUM_PCA9539R_PINS = 16


class Pca9539r(SubSim):
    @classmethod
    def addr_to_key(cls, i2c_address):
        return (stores_pb2.MuStoreType.PCA9539R, i2c_address)

    def update_pin(self, key, pin, state):
        msg = pca9539r_pb2.MuPca9539rStore()
        msg.state[pin] = state
        mask = pca9539r_pb2.MuPca9539rStore()
        mask.state[pin] = 1
        update = StoreUpdate(msg, mask, key)
        self.parent.proj.write_store(update)

    def assert_pin_state(self, key, pin, state):
        assert self.parent.stores[key].state[pin] == state
