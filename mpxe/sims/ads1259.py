from mpxe.protogen import stores_pb2
from mpxe.protogen import ads1259_pb2

import time

from mpxe.sims import sim

ADS1259_KEY = (stores_pb2.MxStoreType.ADS1259, 0)

class Ads1259(sim.Sim):

    def update_ads_reading(self, proj, lsb_val, mid_val, msb_val):
        ads1259_msg = ads1259_pb2.MxAds1259Store()
        ads1259_msg.rx_data_lsb = lsb_val
        ads1259_msg.rx_data_mid = mid_val
        ads1259_msg.rx_data_msb = msb_val

        ads1259_mask = ads1259_pb2.MxAds1259Store()
        ads1259_mask.rx_data_lsb = 1
        ads1259_mask.rx_data_mid = 1
        ads1259_mask.rx_data_msb = 1

        proj.write_store(stores_pb2.MxStoreType.ADS1259, ads1259_msg, ads1259_mask)

    def assert_store_value_lsb(self, proj, reading):
        assert(proj.stores[ADS1259_KEY].rx_data_lsb  == reading) 
    
    def assert_store_value_mid(self, proj, reading):
        assert(proj.stores[ADS1259_KEY].rx_data_mid  == reading) 
    
    def assert_store_value_msb(self, proj, reading):
        assert(proj.stores[ADS1259_KEY].rx_data_msb  == reading) 

