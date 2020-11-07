from mpxe.protogen import stores_pb2
from mpxe.protogen import ads1259_pb2

import time

from mpxe.sims import sim

ADS1259_KEY = (stores_pb2.MxStoreType.ADS1259, 0)

class Ads1259(sim.Sim):

    def update_ads_reading(self, proj, reading):
        ads1259_msg = ads1259_pb2.MxAds1259Store()
        ads1259_msg.reading = reading

        ads1259_mask = ads1259_pb2.MxAds1259Store()
        ads1259_mask.reading = 1
    
        proj.write_store(stores_pb2.MxStoreType.ADS1259, ads1259_msg, ads1259_mask)

    def assert_store_value_reading(self, proj, reading):
        assert(proj.stores[ADS1259_KEY].reading == reading) 
    


