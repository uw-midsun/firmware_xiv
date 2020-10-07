import sys
from os.path import dirname
sys.path.append(dirname(sys.path[0]))

import unittest
import time
from harness import pm
from harness import project

# pm.ProjectManager().build('controller_board_blinking_leds')

class TestLeds(unittest.TestCase):
    def setUp(self):
        self.manager = pm.ProjectManager()
        self.manager.statuses['controller_board_blinking_leds'] = True
        self.leds = self.manager.start('controller_board_blinking_leds')
    
    def tearDown(self):
        self.manager.stop(self.leds)
        self.manager.end()

    def test_leds(self):
        time.sleep(1)

if __name__ == '__main__':
    unittest.main()
