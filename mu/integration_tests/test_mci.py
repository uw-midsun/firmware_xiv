import unittest
import time

from mu.integration_tests import int_test
from mu.sims.mci import Mci
from mu.protogen import stores_pb2

MCP2515_KEY = (stores_pb2.MuStoreType.MCP2515, 0)

class TestMci(int_test.IntTest):
    def setUp(self):
        super().setUp()
        self.board = self.manager.start('mci', Mci)

    def test_mci(self):
        time.sleep(0.3)
        self.assertIsNotNone(self.board.stores[MCP2515_KEY])
        self.assertNotEqual(self.board.stores[MCP2515_KEY].tx_id, 0)
        self.assertEqual(self.board.stores[MCP2515_KEY].tx_data, 0)

        self.manager.can.send('BEGIN_PRECHARGE', None)
        time.sleep(0.4)
        self.assert_can_received('PRECHARGE_COMPLETED')

        self.can_send('PEDAL_OUTPUT', {'throttle_output': 50, 'brake_output': 0})
        time.sleep(0.3)
        self.assertEqual(self.board.stores[MCP2515_KEY].tx_data, 0)

        self.can_send('DRIVE_OUTPUT', {'drive_output': 1})
        time.sleep(0.3)
        self.can_send('PEDAL_OUTPUT', {'throttle_output': 50, 'brake_output': 0})
        time.sleep(0.2)
        self.assertNotEqual(self.board.stores[MCP2515_KEY].tx_data, 0)


if __name__ == '__main__':
    unittest.main()
