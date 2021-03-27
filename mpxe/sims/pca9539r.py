from mpxe.protogen import stores_pb2

from mpxe.sims import sim

NUM_PCA_PINS = 16


class Pca9539r(sim.Sim):
    def __init__(self):
        self.states = {}

    def handle_update(self, pm, proj, key):
        stores = proj.stores
        pca_key = key[1]
        # 0x74 is the address used in the smoke test
        if (stores_pb2.MxStoreType.PCA9539R, pca_key) in stores:
            pca = stores[(stores_pb2.MxStoreType.PCA9539R, pca_key)]
            self.states[pca_key] = [pca.state[i] for i in range(16)]

    def assert_store_values(self, pin, state, pca_key):
        states = self.states[pca_key]
        assert states[pin] == state
