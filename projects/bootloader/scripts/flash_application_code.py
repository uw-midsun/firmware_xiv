# pylint: skip-file
"""This script handles the client side of the flash application code protocol."""

# Import protobuf
import os, sys
currentdir = os.path.dirname(os.path.realpath(__file__))
parentdir = os.path.dirname(currentdir)
sys.path.append(parentdir)

import time
import can
from can_datagram import Datagram, DatagramListener, DatagramSender
from protogen import flash_application_code_pb2 as pb2

class FlashApplication():
    """Flash Application Code Protocol"""

    def __init__(self, board_ids, application_code_data: bytes, channel):
        self.data = application_code_data
        self.board_ids = board_ids
        self.channel = channel

        self.sender = DatagramSender(channel=self.channel, receive_own_messages=False)
        self.listener = DatagramListener(self.listener_callback)
        self.notifier = can.Notifier(self.sender.bus, [self.listener])

    def listener_callback(self, datagram: Datagram, board_id):
        """Check whether the datagram has an OK status code"""

        if datagram._data!= bytearray(0):
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

    def flash_protobuf(self):
        """Flash Protobuf"""

        message = self.create_message(8, pb2.DESCRIPTOR.serialized_pb)
        self.sender.send(message)

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
            return

    def flash_application(self):
        """Flash Application Code"""

        chunked_application_code = self.chunks(self.data, 2048)

        for chunk in chunked_application_code:

            # Reset recieved keys in listener
            self.listener = DatagramListener(self.listener_callback)

            message_chunk = self.create_message(9, chunk)
            self.sender.send(message_chunk)

            self.status_code = 0
            timeout = time.time() + 5
            while not set(self.board_ids) == self.recv_boards:
                self.recv_boards = set(self.listener.datagram_messages.keys())
                if time.time() > timeout:
                    self.status_code = 2
                    return
        return