import unittest
import time

from mpxe.integration_tests import int_test
from mpxe.sims.adt7476a import Adt7476a

class TestAdt7476a(int_test.IntTest):
    def setUp(self):
        super(TestAdt7476a, self).setUp()
        self.adt7476a = self.manager.start('smoke_adt7476a', Adt7476a())

    def test_adt7476a(self):
        time.sleep(1)
        self.adt7476a.sim.assert_store_value(self.adt7476a, 10, 0, 0) # this is for ADT_PWM_PORT_1
        self.adt7476a.sim.assert_store_value(self.adt7476a, 0, 0, 1) # this is for ADT_PWM_PORT_2
        time.sleep(1)
        self.adt7476a.sim.assert_store_value(self.adt7476a, 20, 0, 0) # this is for ADT_PWM_PORT_1
        self.adt7476a.sim.assert_store_value(self.adt7476a, 0, 0, 1) # this is for ADT_PWM_PORT_2
        time.sleep(1)
        self.adt7476a.sim.assert_store_value(self.adt7476a, 30, 0, 0) # this is for ADT_PWM_PORT_1
        self.adt7476a.sim.assert_store_value(self.adt7476a, 0, 0, 1) # this is for ADT_PWM_PORT_2
        time.sleep(1)
        self.adt7476a.sim.assert_store_value(self.adt7476a, 40, 0, 0) # this is for ADT_PWM_PORT_1
        self.adt7476a.sim.assert_store_value(self.adt7476a, 0, 0, 1) # this is for ADT_PWM_PORT_2
        time.sleep(1)
        self.adt7476a.sim.assert_store_value(self.adt7476a, 50, 0, 0) # this is for ADT_PWM_PORT_1
        self.adt7476a.sim.assert_store_value(self.adt7476a, 0, 0, 1) # this is for ADT_PWM_PORT_2
        time.sleep(1)
        self.adt7476a.sim.assert_store_value(self.adt7476a, 60, 0, 0) # this is for ADT_PWM_PORT_1
        self.adt7476a.sim.assert_store_value(self.adt7476a, 0, 0, 1) # this is for ADT_PWM_PORT_2
        time.sleep(1)
        self.adt7476a.sim.assert_store_value(self.adt7476a, 70, 0, 0) # this is for ADT_PWM_PORT_1
        self.adt7476a.sim.assert_store_value(self.adt7476a, 0, 0, 1) # this is for ADT_PWM_PORT_2
        time.sleep(1)
        self.adt7476a.sim.assert_store_value(self.adt7476a, 80, 0, 0) # this is for ADT_PWM_PORT_1
        self.adt7476a.sim.assert_store_value(self.adt7476a, 0, 0, 1) # this is for ADT_PWM_PORT_2
        time.sleep(1)
        self.adt7476a.sim.assert_store_value(self.adt7476a, 90, 0, 0) # this is for ADT_PWM_PORT_1
        self.adt7476a.sim.assert_store_value(self.adt7476a, 0, 0, 1) # this is for ADT_PWM_PORT_2

if __name__ == '__main__':
    unittest.main()
