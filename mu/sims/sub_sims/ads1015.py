from mu.harness.project import StoreUpdate
from mu.harness.sub_sim import SubSim
from mu.protogen import stores_pb2
from mu.protogen import ads1015_pb2

class Ads1015(SubSim):
    def update_reading(self, channel, val):
        ads1015_msg = ads1015_pb2.MuAds1015Store()
        ads1015_msg.readings.extend([0] * 4)
        ads1015_msg.readings[channel] = val

        ads1015_mask = ads1015_pb2.MuAds1015Store()
        ads1015_mask.readings.extend([0] * 4)
        ads1015_mask.readings[channel] = 1
        ads1015_update = StoreUpdate(ads1015_msg, ads1015_mask, stores_pb2.MuStoreType.ADS1015, 0)

        self.parent.proj.write_store(ads1015_update)
