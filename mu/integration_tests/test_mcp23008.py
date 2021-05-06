import unittest
import time

from mu.integration_tests import int_test
from mu.sims.bms_carrier import BmsCarrier
from mu.sims.sub_sims.mcp23008 import NUM_MCP23008_PINS, MCP23008_KEY


class TestMcp23008(int_test.IntTest):
    def setUp(self):
        super().setUp()
        self.board = self.manager.start('smoke_mcp23008', sim_class=BmsCarrier)

    def test_mcp23008(self):
        time.sleep(0.1)
        for x in range(NUM_MCP23008_PINS):  # test all pins init'd
            self.board.sub_sim('mcp23008').assert_pin_state(MCP23008_KEY, x, 1)
        time.sleep(0.1)
        for x in range(NUM_MCP23008_PINS):  # test all pins toggled
            self.board.sub_sim('mcp23008').assert_pin_state(MCP23008_KEY, x, 0)


if __name__ == '__main__':
    unittest.main()
