import unittest
import time

from mu.integration_tests import int_test
from mu.sims.solar import Solar

class TestMcp3427(int_test.IntTest):
    def setUp(self):
        super().setUp()
        self.board = self.manager.start(Solar, proj_name='smoke_mcp3427')

    def test_mcp3427(self):
        time.sleep(0.1)
        # Test set values (only first read in smoketest)
        self.board.sub_sim('mcp3427').update_mcp3427(20, 20, 0)
        time.sleep(1)
        # Test fault callback triggered
        self.board.sub_sim('mcp3427').update_mcp3427(20, 20, 1)
        time.sleep(1)


if __name__ == '__main__':
    unittest.main()
