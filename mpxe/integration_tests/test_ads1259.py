import unittest
import time

from mpxe.integration_tests import int_test
from mpxe.sims.ads1259 import Ads1259

class TestAds1259(int_test.IntTest):
    def setUp(self):
        super(TestAds1259, self).setUp()
        self.ads1259 = self.manager.start('smoke_ads1259', Ads1259())        

    def test_ads1259(self):
        time.sleep(1)
        self.ads1259.sim.update_ads_reading(self.ads1259, 0x9A)       
        # self.assertEqual(self.ads1259.stores["rx_data_lsb"], 0x9A)
        time.sleep(4)

        self.ads1259.sim.read_stores(self.ads1259)
        # self.assertEqual(self.ads1259.sim.set_gpio, 0x9A)
        print("done")

        # print(self.ads1259.stores.items
        # print(self.ads1259.
        # self.assertEqual(self.ads1259.stores["rx_data_msb"], 0x9A)

if __name__ == '__main__':
    unittest.main()
