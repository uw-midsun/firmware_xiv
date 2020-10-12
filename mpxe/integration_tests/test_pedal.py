import sys
import os
from os.path import dirname
sys.path.append(dirname(sys.path[0]))

import unittest
import time
from harness import pm
from harness import project

# pm.ProjectManager().build('pedal_board')

class TestPedal(unittest.TestCase):
    def setUp(self):
        self.manager = pm.ProjectManager()
        # skip build
        self.manager.statuses['pedal_board'] = True
        self.pedal = self.manager.start('pedal_board')

    def tearDown(self):
        self.manager.stop(self.pedal)
        self.manager.end()

    def test_pedal(self):
        time.sleep(0.5)
        # ads1015 reading is zero so throttle should be zero
        msg = self.manager.can.get_latest_by_name('PEDAL_OUTPUT')
        assert(msg.data['throttle_output'] == 0)

        # set throttle channel to 50
        self.pedal.sim.update_ads_reading(self.pedal, 50, 1)
        time.sleep(0.5)
        # ads1015 reading is nonzero so throttle should be nonzero
        msg = self.manager.can.get_latest_by_name('PEDAL_OUTPUT')
        assert(msg.data['throttle_output'] == 50)

if __name__ == '__main__':
    unittest.main()
