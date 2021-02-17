import unittest
import time
import sys
import os

from mpxe.protogen import stores_pb2
from mpxe.protogen import mcp23008_pb2

from mpxe.integration_tests import int_test
from mpxe.integration_tests.init_cond import mcp23008_init_conditions
from mpxe.sims.mcp23008 import Mcp23008, NUM_MCP_PINS

MPXE_INIT_COND = os.getenv('MPXE_INIT_COND')

class TestMcp23008(int_test.IntTest):
    def setUp(self):
        super().setUp()
        if MPXE_INIT_COND == "True":
            self.mcp23008 = self.manager.start('smoke_mcp23008', Mcp23008(), mcp23008_init_conditions())        
            print("here")
        else:
            print("Heere 2")
            self.mcp23008 = self.manager.start('smoke_mcp23008', Mcp23008())        

    def test_mcp23008(self):
        time.sleep(0.1)
        for x in range(NUM_MCP_PINS): # test all pins init'd
            self.mcp23008.sim.assert_store_value_reading(self.mcp23008, x, 1)
        time.sleep(0.1)
        for x in range(NUM_MCP_PINS): # test all pins toggled
            self.mcp23008.sim.assert_store_value_reading(self.mcp23008, x, 0)            

if __name__ == '__main__':
    unittest.main()
