from mu.protogen import stores_pb2
from mu.protogen import ads1259_pb2
from mu.harness.project import StoreUpdate

from mu.sims import sim

ADS1259_KEY = (stores_pb2.MuStoreType.ADS1259, 0)


class Ads1259(sim.Sim):

    def update_ads_reading(self, proj, reading):
        ads1259_msg = ads1259_pb2.MuAds1259Store()
        ads1259_msg.reading = reading

        ads1259_mask = ads1259_pb2.MuAds1259Store()
        ads1259_mask.reading = 1
        ads1259_update = StoreUpdate(ads1259_msg, ads1259_mask, stores_pb2.MuStoreType.ADS1259, 0)
        proj.write_store(ads1259_update)

    def assert_store_value_reading(self, proj, reading):
        # as there is no export to store for this driver this will be false
        if ADS1259_KEY in proj.stores:
            assert proj.stores[ADS1259_KEY].reading == reading
