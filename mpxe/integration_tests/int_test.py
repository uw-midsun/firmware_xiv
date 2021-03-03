import unittest

from mpxe.harness.pm import ProjectManager

class IntTest(unittest.TestCase):
    def setUp(self):
        print("Running test", self._testMethodName)
        self.manager = ProjectManager()

    def tearDown(self):
        self.manager.end()
        print("======", "PASS", self._testMethodName, "======")

    def assert_can_data(self, name, field, val):
        msg = self.manager.can.get_latest_by_name(name)
        self.assertIsNotNone(msg)
        self.assertEqual(msg.data[field], val)

    def assert_can_received(self, name):
        self.assertIsNotNone(self.manager.can.get_latest_by_name(name))

    def can_send(self, name, data):
        self.manager.can.send(name, data)
