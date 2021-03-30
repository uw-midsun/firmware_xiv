import unittest

from mpxe.integration_tests import int_test
# ask to add a simulation or not


class TestBabyDriver(int_test.IntTest):
    def setUp(self):
        super().setUp()
        self.babydriver = self.manager.start('baby_driver')
