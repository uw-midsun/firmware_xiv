import unittest
import time

from mpxe.integration_tests import int_test

class TestAdt(int_test.IntTest):
    def setUp(self):
        super(TestAdt, self).setUp()
        # self.adt = self.manager.start('pedal_board', PedalBoard())

    def test_adt(self):
        print("hello world")

if __name__ == '__main__':
    unittest.main()
