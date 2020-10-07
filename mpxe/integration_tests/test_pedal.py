import sys
import os
from os.path import dirname
sys.path.append(dirname(sys.path[0]))

import unittest
import warnings
import time
from harness import pm
from harness import project

class TestPedal(unittest.TestCase):
    def setUp(self):
        self.manager = pm.ProjectManager()
        self.manager.statuses['pedal_board'] = True
        self.pedal = self.manager.start('pedal_board')

    def tearDown(self):
        self.manager.stop(self.pedal)
        self.manager.end()

    def test_pedal(self):
        time.sleep(0.5)

        msg = self.manager.can.get_latest_by_name('PEDAL_OUTPUT')
        assert(msg.data['throttle_output'] == 0)

        self.pedal.handler.update_ads_reading(self.pedal, 50, 1)
        time.sleep(0.5)
        msg = self.manager.can.get_latest_by_name('PEDAL_OUTPUT')
        assert(msg.data['throttle_output'] == 50)

if __name__ == '__main__':
    warnings.filterwarnings(action='ignore', message='unclosed', category=ResourceWarning)
    # pm.ProjectManager().build('pedal_board')
    unittest.main()
