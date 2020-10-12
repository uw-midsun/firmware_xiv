import sys
from os.path import dirname
sys.path.append(dirname(sys.path[0]))

import unittest
import time
from harness import pm
from harness import project

# pm.ProjectManager().build('can_dump')

class TestCanDump(unittest.TestCase):
    def setUp(self):
        self.manager = pm.ProjectManager()
        # skip build
        self.manager.statuses['can_dump'] = True
        self.can_dump = self.manager.start('can_dump')
    
    def tearDown(self):
        self.manager.stop(self.can_dump)
        self.manager.end()

    def test_can_dump(self):
        time.sleep(1)

if __name__ == '__main__':
    unittest.main()
