import sys
from os.path import dirname
sys.path.append(dirname(sys.path[0]))

import unittest
import time

import int_test

# pm.ProjectManager().build('can_dump')

class TestCanDump(int_test.IntTest):
    def setUp(self):
        super(TestCanDump, self).setUp()
        self.can_dump = self.manager.start('can_dump')

    def test_can_dump(self):
        time.sleep(1)

if __name__ == '__main__':
    unittest.main()
