from mpxe.protogen import stores_pb2
from mpxe.protogen import pca9539r_pb2

from mpxe.sims import sim

class Pca9539r(sim.Sim):
    def handle_update(self, pm, proj):
        stores = proj.stores
        if (stores_pb2.MxStoreType.PCA9539R, 116) in stores:    #116 is the address used in the smoke test
            pca        = stores[(stores_pb2.MxStoreType.PCA9539R, 116)]
            self.state = [pca.state[i] for i in range(16)]

    def assert_store_values(self, test, state):
        for x in range(16):
            test.assertEqual(self.state[x], state)
