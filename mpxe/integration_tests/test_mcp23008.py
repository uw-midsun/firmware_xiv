import unittest
import time

from mpxe.integration_tests import int_test
from mpxe.sims.mcp23008 import Mcp23008, NUM_MCP_PINS

class TestMcp23008(int_test.IntTest):
    def setUp(self):
        super().setUp()
        self.mcp23008 = self.manager.start('smoke_mcp23008', Mcp23008())        

    def test_mcp23008(self):
        for x in range(NUM_MCP_PINS): # test all pins init'd
            time.sleep(1)     
            self.mcp23008.sim.assert_store_value_reading(self.mcp23008, x, 1)
        time.sleep(3) # allow check pins to run
        for x in range(NUM_MCP_PINS): # test toggle pins
            time.sleep(1)     
            self.mcp23008.sim.assert_store_value_reading(self.mcp23008, x, 0)

if __name__ == '__main__':
    unittest.main()
