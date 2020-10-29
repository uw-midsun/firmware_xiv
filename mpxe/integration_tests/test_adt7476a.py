import unittest
import time

from mpxe.integration_tests import int_test
from mpxe.sims.adt7476a import Adt7476a

class TestAdt7476a(int_test.IntTest):
    def setUp(self):
        super(TestAdt7476a, self).setUp()
        self.adt7476a = self.manager.start('smoke_adt7476a', Adt7476a())

    def test_adt7476a(self):
        print("hello world")
        time.sleep(10)

if __name__ == '__main__':
    unittest.main()
