from ..protogen import stores_pb2
from ..protogen import ads1015_pb2
from .mock_helpers import write_proj

class PedalBoard:
    def handle_update(self, pm, proj):
        pass
    def handle_log(self, pm, proj, log):
        print('[{}]'.format(proj.name), log)
    def update_ads_reading(self, proj, val, channel):
        ads1015_msg = ads1015_pb2.MxAds1015Store()
        ads1015_msg.readings.extend([0] * 4)
        ads1015_msg.readings[channel] = val

        ads1015_mask = ads1015_pb2.MxAds1015Store()
        ads1015_mask.readings.extend([0] * 4)
        ads1015_mask.readings[channel] = 1

        write_proj(proj, stores_pb2.MxStoreType.ADS1015, ads1015_msg, ads1015_mask)
