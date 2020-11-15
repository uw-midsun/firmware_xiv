import unittest
import time

from mpxe.integration_tests import int_test

class TestCanDump(int_test.IntTest):
    def setUp(self):
        super().setUp()
        self.can_dump = self.manager.start('can_dump')

    def test_can_dump(self):
        time.sleep(0.1)

if __name__ == '__main__':
    unittest.main()
