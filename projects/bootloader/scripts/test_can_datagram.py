# pylint: skip-file
"""This Module Tests methods in can_util.py"""
import unittest
import can

from can_datagram import Datagram
from can_datagram import DatagramSender
from can_datagram import DatagramListener

DEFAULT_CHANNEL = "vcan0"

TEST_PROTOCOL_VERSION = 0
TEST_DATAGRAM_TYPE_ID = 1
TEST_NODES = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
TEST_DATA = [3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5, 8, 9, 7, 9, 3, 2, 3, 8, 4, 6, 2, 6, 4, 3, 3]


class TestCanDatagram(unittest.TestCase):
    """Test Can Datagram functions"""

    def test_create_message(self):
        """Test the constructor of the Datagram class"""

        message = Datagram(
            protocol_version=TEST_PROTOCOL_VERSION,
            datagram_type_id=TEST_DATAGRAM_TYPE_ID,
            node_ids=TEST_NODES,
            data=bytearray(TEST_DATA))

        self.assertEqual(message.get_protocol_version(), TEST_PROTOCOL_VERSION)
        self.assertEqual(message.get_datagram_type_id(), TEST_DATAGRAM_TYPE_ID)
        self.assertEqual(message.get_node_ids(), TEST_NODES)
        self.assertEqual(message.get_data(), bytearray(TEST_DATA))

    def test_modify_message(self):
        """Test modifying values in the Datagram class"""
        message = Datagram(
            protocol_version=TEST_PROTOCOL_VERSION,
            datagram_type_id=TEST_DATAGRAM_TYPE_ID,
            node_ids=TEST_NODES,
            data=bytearray(TEST_DATA))

        self.assertEqual(message.get_protocol_version(), TEST_PROTOCOL_VERSION)
        self.assertEqual(message.get_datagram_type_id(), TEST_DATAGRAM_TYPE_ID)
        self.assertEqual(message.get_node_ids(), TEST_NODES)
        self.assertEqual(message.get_data(), bytearray(TEST_DATA))

        protocol_version = 9
        datagram_type_id = 8
        test_nodes = list(reversed(TEST_NODES))
        test_data = list(reversed(TEST_DATA))

        message.set_protocol_version(protocol_version)
        message.set_datagram_type_id(datagram_type_id)
        message.set_node_ids(test_nodes)
        message.set_data(bytearray(test_data))

        self.assertEqual(message.get_protocol_version(), protocol_version)
        self.assertEqual(message.get_datagram_type_id(), datagram_type_id)
        self.assertEqual(message.get_node_ids(), test_nodes)
        self.assertEqual(message.get_data(), bytearray(test_data))

    def test_serialize(self):
        """Test the serialize function"""
        message = Datagram(
            protocol_version=TEST_PROTOCOL_VERSION,
            datagram_type_id=TEST_DATAGRAM_TYPE_ID,
            node_ids=TEST_NODES,
            data=bytearray(TEST_DATA))

        self.assertEqual(message.serialize(), bytearray(
            b'\x00\x00\x00\x00\x00\x01\n\x01\x02\x03\x04\x05\x06\x07\x08\t\n\x01\n\x03\x01\x04\x01\x05\t\x02\x06\x05\x03\x05\x08\t\x07\t\x03\x02\x03\x08\x04\x06\x02\x06\x04\x03\x03'))

    def test_deserialize(self):
        """Test retrieving Datagram information from the bytearray"""
        message = Datagram(
            protocol_version=0,
            datagram_type_id=0,
            node_ids=[0],
            data=bytearray(
                [0]))

        message.deserialize(bytearray(
            b'\x00\x00\x00\x00\x00\x01\n\x01\x02\x03\x04\x05\x06\x07\x08\t\n\x01\n\x03\x01\x04\x01\x05\t\x02\x06\x05\x03\x05\x08\t\x07\t\x03\x02\x03\x08\x04\x06\x02\x06\x04\x03\x03'))

        self.assertEqual(message.get_protocol_version(), TEST_PROTOCOL_VERSION)
        self.assertEqual(message.get_datagram_type_id(), TEST_DATAGRAM_TYPE_ID)
        self.assertEqual(message.get_node_ids(), TEST_NODES)
        self.assertEqual(message.get_data(), bytearray(TEST_DATA))


if __name__ == '__main__':
    unittest.main()
