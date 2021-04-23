from mu.harness.project import StoreUpdate
from mu.harness.sub_sim import SubSim
from mu.protogen import stores_pb2
from mu.protogen import ads1259_pb2

ADS1259_KEY = (stores_pb2.MuStoreType.ADS1259, 0)

class Ads1259(SubSim):
    def update_reading(self, reading):
        ads1259_msg = ads1259_pb2.MuAds1259Store()
        ads1259_msg.reading = reading

        ads1259_mask = ads1259_pb2.MuAds1259Store()
        ads1259_mask.reading = 1
        ads1259_update = StoreUpdate(ads1259_msg, ads1259_mask, ADS1259_KEY)
        self.parent.proj.write_store(ads1259_update)
