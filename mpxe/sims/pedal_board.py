from mpxe.protogen import stores_pb2
from mpxe.protogen import ads1015_pb2

from mpxe.sims import sim

class PedalBoard(sim.Sim):
    def update_ads_reading(self, proj, val, channel):
        ads1015_msg = ads1015_pb2.MxAds1015Store()
        ads1015_msg.readings.extend([0] * 4)
        ads1015_msg.readings[channel] = val

        ads1015_mask = ads1015_pb2.MxAds1015Store()
        ads1015_mask.readings.extend([0] * 4)
        ads1015_mask.readings[channel] = 1

        proj.write_store(ads1015_msg, ads1015_mask, stores_pb2.MxStoreType.ADS1015)
