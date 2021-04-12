import unittest
import time

from mpxe.integration_tests import int_test
from mpxe.sims.mci import Mci


class TestMci(int_test.IntTest):
    def setUp(self):
        super().setUp()
        self.mci = self.manager.start('mci', Mci())

    def test_mci(self):
        time.sleep(0.3)
        self.assertNotEqual(self.mci.sim.tx_id, 0)
        self.assertEqual(self.mci.sim.tx_data, 0)

        self.manager.can.send('BEGIN_PRECHARGE', None)
        time.sleep(0.4)
        self.assert_can_received('PRECHARGE_COMPLETED')

        self.can_send('PEDAL_OUTPUT', {'throttle_output': 50, 'brake_output': 0})
        time.sleep(0.3)
        self.assertEqual(self.mci.sim.tx_data, 0)

        self.can_send('DRIVE_OUTPUT', {'drive_output': 1})
        time.sleep(0.3)
        self.can_send('PEDAL_OUTPUT', {'throttle_output': 50, 'brake_output': 0})
        time.sleep(0.3)
        self.assertNotEqual(self.mci.sim.tx_data, 0)


if __name__ == '__main__':
    unittest.main()
