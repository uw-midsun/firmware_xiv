import unittest
import time

from mpxe.integration_tests import int_test
from mpxe.sims.adt7476a import Adt7476a

class TestAdt7476a(int_test.IntTest):
    def setUp(self):
        super(TestAdt7476a, self).setUp()
        self.adt7476a = self.manager.start('smoke_adt7476a', Adt7476a())

    def test_adt7476a(self):
        for x in range(1, 10):
            time.sleep(1)
            self.adt7476a.sim.assert_store_values(self.adt7476a, x * 10, 0, 0) # this is for ADT_PWM_PORT_1
            self.adt7476a.sim.assert_store_values(self.adt7476a, 0, 0, 1) # this is for ADT_PWM_PORT_2
            

if __name__ == '__main__':
    unittest.main()
