import unittest

from mu.harness.pm import ProjectManager
from mu.srv.config import Config


class IntTest(unittest.TestCase):
    def setUp(self):
        print("Running test", self._testMethodName)
        config = Config(False, 'vcan0', True)
        self.manager = ProjectManager(config)

    def tearDown(self):
        self.manager.end()
        print("======", "DONE", self._testMethodName, "======")

    def assert_can_data(self, name, field, val):
        msg = self.manager.can.get_latest_by_name(name)
        self.assertIsNotNone(msg)
        self.assertEqual(msg.data[field], val)

    def assert_can_received(self, name):
        self.assertIsNotNone(self.manager.can.get_latest_by_name(name))

    def can_send(self, name, data):
        self.manager.can.send(name, data)
