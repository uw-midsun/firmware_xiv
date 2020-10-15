import unittest
import time
from mpxe.harness import pm
from mpxe.harness import project

class IntTest(unittest.TestCase):
    def setUp(self):
        print("Running test", self._testMethodName)
        self.manager = pm.ProjectManager()
    
    def tearDown(self):
        self.manager.end()
        print("======", "PASS", self._testMethodName, "======")

    def assert_can_data(self, name, field, val):
        msg = self.manager.can.get_latest_by_name(name)
        self.assertIsNotNone(msg)
        self.assertEqual(msg.data[field], val)

    def assert_can_got(self, name):
        self.assertIsNotNone(self.manager.can.get_latest_by_name(name))

    def can_send(self, name, data):
        self.manager.can.send(name, data)
