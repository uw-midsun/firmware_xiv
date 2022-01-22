# pylint: skip-file
"""This Module Tests methods in can_util.py"""
import unittest
import can
import asyncio
import time

from can_datagram import Datagram
from can_datagram import DatagramSender
from can_datagram import DatagramListener

TEST_CHANNEL = "vcan0"

TEST_PROTOCOL_VERSION = 1
TEST_DATAGRAM_TYPE_ID = 1
TEST_NODES = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
TEST_DATA = [3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5, 8, 9, 7, 9, 3, 2, 3, 8, 4, 6, 2, 6, 4, 3, 3]


class TestCanDatagram(unittest.TestCase):
    """Test Can Datagram functions"""

    def test_create_message(self):
        """Test the constructor of the Datagram class"""

        message = Datagram(
            datagram_type_id=TEST_DATAGRAM_TYPE_ID,
            node_ids=TEST_NODES,
            data=bytearray(TEST_DATA))

        self.assertEqual(message._protocol_version, TEST_PROTOCOL_VERSION)
        self.assertEqual(message._datagram_type_id, TEST_DATAGRAM_TYPE_ID)
        self.assertEqual(message._node_ids, TEST_NODES)
        self.assertEqual(message._data, bytearray(TEST_DATA))

    def test_modify_message(self):
        """Test modifying values in the Datagram class"""
        message = Datagram(
            protocol_version=TEST_PROTOCOL_VERSION,
            datagram_type_id=TEST_DATAGRAM_TYPE_ID,
            node_ids=TEST_NODES,
            data=bytearray(TEST_DATA))

        self.assertEqual(message._protocol_version, TEST_PROTOCOL_VERSION)
        self.assertEqual(message._datagram_type_id, TEST_DATAGRAM_TYPE_ID)
        self.assertEqual(message._node_ids, TEST_NODES)
        self.assertEqual(message._data, bytearray(TEST_DATA))

        protocol_version = 9
        datagram_type_id = 8
        test_nodes = list(reversed(TEST_NODES))
        test_data = list(reversed(TEST_DATA))

        message.protocol_version = protocol_version
        message.datagram_type_id = datagram_type_id
        message.node_ids = test_nodes
        message.data = bytearray(test_data)

        self.assertEqual(message._protocol_version, protocol_version)
        self.assertEqual(message._datagram_type_id, datagram_type_id)
        self.assertEqual(message._node_ids, test_nodes)
        self.assertEqual(message._data, bytearray(test_data))

    def test_serialize(self):
        """Test the serialize function"""
        message = Datagram(
            protocol_version=TEST_PROTOCOL_VERSION,
            datagram_type_id=TEST_DATAGRAM_TYPE_ID,
            node_ids=TEST_NODES,
            data=bytearray(TEST_DATA))

        bytes = bytearray([1,                   # Datagram Protocol Version
                           109, 194, 24, 254,  # CRC32, little-endian
                           1,                  # Datagram Type ID
                           10,                 # Number of Node ID's
                           *TEST_NODES,        # Node ID's
                           26, 0,              # Data Size, little-endian
                           *TEST_DATA          # Data
                           ])

        self.assertEqual(message.serialize(), bytes)

    def test_deserialize(self):
        """Test retrieving Datagram information from the bytearray"""
        bytes = bytearray([1,                   # Datagram Protocol Version
                           109, 194, 24, 254,  # CRC32, little-endian
                           1,                  # Datagram Type ID
                           10,                 # Number of Node ID's
                           *TEST_NODES,        # Node ID's
                           26, 0,              # Data Size, little-endian
                           *TEST_DATA          # Data
                           ])

        message = Datagram.deserialize(bytes)
        self.assertEqual(message._protocol_version, TEST_PROTOCOL_VERSION)
        self.assertEqual(message._datagram_type_id, TEST_DATAGRAM_TYPE_ID)
        self.assertEqual(message._node_ids, TEST_NODES)
        self.assertEqual(message._data, bytearray(TEST_DATA))


class TestCanDatagramSender(unittest.TestCase):
    """Test Can Datagram functions"""

    def test_send_message(self):
        """Test the distributor of the Datagram class"""

        sender = DatagramSender(channel=TEST_CHANNEL, receive_own_messages=True)

        listener = can.BufferedReader()
        notifier = can.Notifier(sender.bus, [listener])

        message = Datagram(
            datagram_type_id=TEST_DATAGRAM_TYPE_ID,
            node_ids=TEST_NODES,
            data=bytearray(TEST_DATA))
        sender.send(message)

        recv_datagram = []
        listener_message = listener.get_message()
        while listener_message is not None:
            for byte in listener_message.data:
                recv_datagram.append(byte)
            listener_message = listener.get_message()

        self.assertEqual(message.serialize(), bytearray(recv_datagram))


class TestCanDatagramListener(unittest.TestCase):
    """Test Can Datagram functions"""

    def test_register_callback(self):
        """Test the registering of a callback"""
        self.callback_triggered = False
        self.message = []

        sender = DatagramSender(channel=TEST_CHANNEL, receive_own_messages=True)
        listener = DatagramListener(self.triggerCallback)
        notifier = can.Notifier(sender.bus, [listener])

        message = Datagram(
            datagram_type_id=TEST_DATAGRAM_TYPE_ID,
            node_ids=TEST_NODES,
            data=bytearray(TEST_DATA))
        sender.send(message)

        timeout = time.time() + 10
        while not self.callback_triggered:
            if time.time() > timeout:
                break

        self.assertEqual(self.message.serialize(), message.serialize())
        self.assertEqual(self.callback_triggered, True)

    def triggerCallback(self, msg, board_id):
        self.message = msg
        self.callback_triggered = True


if __name__ == '__main__':
    unittest.main()
