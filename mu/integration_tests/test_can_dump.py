import unittest
import time

from mu.integration_tests import int_test
from mu.harness.board_sim import BoardSim


class TestCanDump(int_test.IntTest):
    def setUp(self):
        super().setUp()
        self.board = self.manager.start(BoardSim, proj_name='can_dump')

    def test_can_dump(self):
        time.sleep(0.1)
        self.assert_can_data('UNKNOWN', 'data', [])


if __name__ == '__main__':
    unittest.main()
