import sys
import os
from os.path import dirname
sys.path.append(dirname(sys.path[0]))

import unittest
import time
from harness import pm
from harness import project

class IntTest(unittest.TestCase):
    def setUp(self):
        self.manager = pm.ProjectManager()
    
    def tearDown(self):
        self.manager.end()

    def assert_can_data(self, name, field, val):
        msg = self.manager.can.get_latest_by_name(name)
        self.assertIsNotNone(msg)
        self.assertEqual(msg.data[field], val)

    def assert_can_got(self, name):
        self.assertIsNotNone(self.manager.can.get_latest_by_name(name))

    def can_send(self, name, data):
        self.manager.can.send(name, data)
