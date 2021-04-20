import unittest
import time

from mu.integration_tests import int_test
from mu.sims.bms_carrier import BmsCarrier


class TestAdt7476a(int_test.IntTest):
    def setUp(self):
        super().setUp()
        self.board = self.manager.start('smoke_adt7476a', sim_class=BmsCarrier)

    def test_adt7476a(self):
        for x in range(1, 3):
            time.sleep(0.1)
            # this is for ADT_PWM_PORT_1
            self.board.sub_sims['Adt7476a'].assert_values(x * 10, 0, 0)
            # this is for ADT_PWM_PORT_2
            self.board.sub_sims['Adt7476a'].assert_values(0, 0, 1)


if __name__ == '__main__':
    unittest.main()
