"""This script handles the client side of the flash application code protocol."""

# pylint: disable=C0411
# pylint: disable=C0413
# pylint: disable=W0212
# autopep8: off

from can_datagram import Datagram, DatagramListener, DatagramSender
import can
import zlib as zl
import time
import os
import sys
# Import protobuf
currentdir = os.path.dirname(os.path.realpath(__file__))
parentdir = os.path.dirname(currentdir)
sys.path.append(parentdir)
from protogen import flash_application_code_pb2 as pb2

OUTGOING_PROTOBUF_ID = 8
OUTGOING_DATA_ID = 9

STATUS_CODE_OKAY = 0
STATUS_CODE_ERROR = 1
STATUS_CODE_TIMEOUT = 2

# pylint: disable=too-many-arguments
def flash_application_code(
    channel,
    application_code_data,
    board_ids,
    name,
    git_version,
    sender_receive_own_messages=False
):
    """Full Flash Application Code Function"""

    flash = FlashApplication(
        channel,
        application_code_data,
        board_ids,
        name,
        git_version,
        sender_receive_own_messages
    )

    flash.flash_protobuf()
    chunked_application_code = flash.chunks(flash.app_data, 2048)
    for chunk in chunked_application_code:
        # Reset received keys in listener
        flash.listener.datagram_messages = {}
        flash.recv_boards = set()
        flash.flash_application_chunk(chunk)


class FlashApplication:
    """Flash Application Code Protocol"""

    # pylint: disable=too-many-instance-attributes
    # pylint: disable=too-many-arguments
    def __init__(
        self,
        channel,
        application_code_data,
        board_ids,
        name,
        git_version,
        sender_receive_own_messages=False
    ):

        self.channel = channel
        self.app_data = application_code_data
        self.board_ids = board_ids
        self.name = name
        self.git_version = git_version

        self.sender = DatagramSender(
            channel=self.channel,
            receive_own_messages=sender_receive_own_messages
        )
        self.listener = DatagramListener(self.listener_callback)
        self.notifier = can.Notifier(self.sender.bus, [self.listener])

        self.recv_boards = set()
        self.status_code = 0

    def listener_callback(self, datagram: Datagram, board_id):
        """Check whether the datagram has an OK status code"""
        self.recv_boards.add(board_id)
        # Ignores non-zero codes from the client
        if datagram._data != bytearray([0]) and board_id != 0:
            self.status_code = STATUS_CODE_ERROR
        elif board_id == 0:
            # Mock controller board responses
            # Only runs when testing - client id is 0
            for node_id in self.board_ids:
                if node_id == 0:
                    continue
                mock_msg = Datagram(
                    datagram_type_id=0,
                    node_ids=self.board_ids,
                    data=bytearray([0]))
                self.sender.send(mock_msg, node_id)

    def chunks(self, app_code: bytearray, chunk_size):
        """Yield successive chunk_size byte-sized chunks from app_code."""
        for i in range(0, len(app_code), chunk_size):
            yield app_code[i:i + chunk_size]

    def create_message(self, datagram_type_id, data):
        """Creates a datagram to be sent with a DatagramSender"""

        message = Datagram(
            datagram_type_id=datagram_type_id,
            node_ids=self.board_ids,
            data=bytearray(data))
        return message

    def flash_protobuf(self):
        """Flash Protobuf"""

        # Construct and serialize protobuf
        protobuf = pb2.FlashApplicationCode()
        protobuf.name = self.name
        protobuf.git_version = bytes(self.git_version)
        protobuf.application_crc = zl.crc32(self.app_data)
        protobuf.size = sys.getsizeof(self.app_data)
        serialized = protobuf.SerializeToString()

        # Send protobuf
        message = self.create_message(OUTGOING_PROTOBUF_ID, serialized)
        self.sender.send(message)

        self.status_code = 0
        timeout = time.time() + 5
        self.recv_boards = set()
        while set(self.board_ids) != self.recv_boards:
            if time.time() > timeout:
                self.status_code = STATUS_CODE_TIMEOUT
                raise Exception("Flash Application Failed - Timeout")

        if self.status_code != 0:
            raise Exception("Flash Application Failed - Non-zero status code")

    def flash_application_chunk(self, chunk):
        """Flash Application Code Chunk"""

        self.recv_boards = set()

        message_chunk = self.create_message(OUTGOING_DATA_ID, chunk)
        self.sender.send(message_chunk)

        self.status_code = 0
        timeout = time.time() + 5
        while set(self.board_ids) != self.recv_boards:
            if time.time() > timeout:
                self.status_code = STATUS_CODE_TIMEOUT
                raise Exception("Flash Application Failed - Timeout")

        if self.status_code != 0:
            raise Exception("Flash Application Failed - Non-zero status code")
