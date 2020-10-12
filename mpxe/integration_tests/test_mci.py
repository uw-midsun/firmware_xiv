import sys
import os
from os.path import dirname
sys.path.append(dirname(sys.path[0]))

import unittest
import warnings
import time
from harness import pm
from harness import project

class TestMci(unittest.TestCase):
    def setUp(self):
        self.manager = pm.ProjectManager()
        self.manager.statuses['mci'] = True
        self.mci = self.manager.start('mci')

    def tearDown(self):
        self.manager.stop(self.mci)
        self.manager.end()
    
    def test_mci(self):
        time.sleep(1)
        assert(self.mci.sim.tx_id != 0)
        assert(self.mci.sim.tx_data == 0)

        self.manager.can.send('BEGIN_PRECHARGE', None)
        time.sleep(1)
        assert(self.manager.can.get_latest_by_name('PRECHARGE_COMPLETED') != None)

        self.manager.can.send('PEDAL_OUTPUT', {'throttle_output': 50, 'brake_output': 0})
        time.sleep(1)
        assert(self.mci.sim.tx_data == 0)

        self.manager.can.send('DRIVE_OUTPUT', {'drive_output': 1})
        time.sleep(1)
        self.manager.can.send('PEDAL_OUTPUT', {'throttle_output': 50, 'brake_output': 0})
        time.sleep(1)
        assert(self.mci.sim.tx_data != 0)

if __name__ == '__main__':
    warnings.filterwarnings(action='ignore', message='unclosed', category=ResourceWarning)
    pm.ProjectManager().build('mci')
    unittest.main()
