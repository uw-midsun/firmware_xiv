# pylint: skip-file
"""This Module Tests methods in flash_application_code.py"""

# Import protobuf
import os, sys
currentdir = os.path.dirname(os.path.realpath(__file__))
parentdir = os.path.dirname(currentdir)
sys.path.append(parentdir)
from protogen import flash_application_code_pb2 as pb2

import unittest
import can
import asyncio
import time

from can_datagram import Datagram
from can_datagram import DatagramSender
from can_datagram import DatagramListener

TEST_PROTOCOL_VERSION = 1
TEST_DATAGRAM_TYPE_ID = 1
TEST_NODES = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
TEST_DATA = [3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5, 8, 9, 7, 9, 3, 2, 3, 8, 4, 6, 2, 6, 4, 3, 3]
TEST_CHANNEL = "vcan0"

class TestFlashApplication(unittest.TestCase):
  """Protocol edited to mock response datagrams"""

  def __init__(self, board_ids, application_code_data: bytes, channel):
    self.data = application_code_data
    self.board_ids = board_ids
    self.channel = channel

    self.sender = DatagramSender(channel=self.channel, receive_own_messages=False)
    self.listener = DatagramListener(self.listener_callback)
    self.notifier = can.Notifier(self.sender.bus, [self.listener])

    self.mock_sender = DatagramSender(channel=self.channel, receive_own_messages=True)

  def listener_callback(self, datagram: Datagram, board_id):
    """Check whether the datagram has an OK status code"""
    if datagram._data!= bytearray(0) or datagram._datagram_type_id != 0:
        self.status_code = 1

  def chunks(self, app_code: bytearray, chunk_size):
    """Yield successive chunk_size byte-sized chunks from app_code."""
    for i in range(0, len(app_code), chunk_size):
        yield app_code[i:i + chunk_size]

  def create_message(self, id, data):
    message = Datagram(
        datagram_type_id=id,
        node_ids=self.board_ids,
        data=bytearray(data))
    return message

  def mock_responses(self):
    """Mocks board responses for testing"""
    for node_id in self.board_ids:
        mock_msg = Datagram(
            datagram_type_id=0,
            node_ids=self.board_ids,
            data=bytearray(0))
        self.mock_sender.send(mock_msg, node_id)

  def flash_protobuf(self):
    """Flash Protobuf"""

    message = self.create_message(8, pb2.DESCRIPTOR.serialized_pb)
    self.sender.send(message)

    self.mock_responses()

    self.status_code = 0
    timeout = time.time() + 5
    self.recv_boards = set()
    while not set(self.board_ids) == self.recv_boards:
        self.recv_boards = set(self.listener.datagram_messages.keys())
        if time.time() > timeout:
            self.status_code = 2
            break
    
    # Return if status code fails
    if self.status_code != 0:
        return False
    return True

  def flash_application(self):
    """Flash Application Code"""

    chunked_application_code = self.chunks(self.data, 2048)

    for chunk in chunked_application_code:

        # Reset recieved keys in listener
        self.listener = DatagramListener(self.listener_callback)

        message_chunk = self.create_message(9, chunk)
        self.sender.send(message_chunk)

        self.mock_responses()

        self.status_code = 0
        timeout = time.time() + 5
        while not set(self.board_ids) == self.recv_boards:
            self.recv_boards = set(self.listener.datagram_messages.keys())
            if time.time() > timeout:
                self.status_code = 2
                return
    return

class TestFAC(unittest.TestCase):
  """Test Flash Application Code functions"""

  def test_flash_application(self):
    """Tests"""
    self.flash = TestFlashApplication(TEST_NODES, bytes(TEST_DATA), TEST_CHANNEL)

    status = self.flash.flash_protobuf()
    self.assertEqual(status, True)

    expected_nodes = set(TEST_NODES)

    self.assertEqual(self.flash.recv_boards, expected_nodes)
    self.assertEqual(self.flash.status_code, 0)

    self.flash.flash_application()

    self.assertEqual(self.flash.recv_boards, expected_nodes)
    self.assertEqual(self.flash.status_code, 0)

if __name__ == '__main__':
    unittest.main()
