from mpxe.protogen import stores_pb2
from mpxe.protogen import ads1259_pb2

import time

from mpxe.sims import sim

ADS1259_KEY = (stores_pb2.MxStoreType.ADS1259, 0)

class Ads1259(sim.Sim):
    def read_stores(self, proj):
        
        if ADS1259_KEY in proj.stores:
            return proj.stores[ADS1259_KEY].rx_data_lsb

    def update_ads_reading(self, proj, val):
        ads1259_msg = ads1259_pb2.MxAds1259Store()
        ads1259_msg.rx_data_lsb = val
        ads1259_msg.rx_data_mid = val
        ads1259_msg.rx_data_msb = val

        ads1259_mask = ads1259_pb2.MxAds1259Store()
        ads1259_mask.rx_data_lsb = val
        ads1259_mask.rx_data_mid = val
        ads1259_mask.rx_data_msb = val

        proj.write_store(stores_pb2.MxStoreType.ADS1259, ads1259_msg, ads1259_mask)
