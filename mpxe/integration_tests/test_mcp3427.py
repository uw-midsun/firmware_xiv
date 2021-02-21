import unittest
import time

from mpxe.integration_tests import int_test
from mpxe.sims.mcp3427 import Mcp3427


class TestMcp3427(int_test.IntTest):
    def setUp(self):
        super().setUp()
        self.mcp3427 = self.manager.start('smoke_mcp3427', Mcp3427())

    def test_mcp3427(self):
        time.sleep(1)
        self.mcp3427.sim.assert_store_value_reading(self.mcp3427, 20)
        time.sleep(1)
        self.mcp3427.sim.update_mcp3427(self.mcp3427, 1)


if __name__ == '__main__':
    unittest.main()