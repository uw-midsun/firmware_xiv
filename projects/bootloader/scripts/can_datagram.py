# pylint: skip-file
"""This client script handles datagram protocol communication between devices on the CAN."""

import zlib
import can

DEFAULT_CHANNEL = "can0"
PROT_VER = 1
CAN_BITRATE = 500000

MESSAGE_SIZE = 8
HEADER_SIZE = 6
MIN_BYTEARRAY_SIZE = 9
DATA_SIZE_SIZE = 2

PROTOCOL_VERSION_OFFSET = 0
CRC_32_OFFSET = 1
DATAGRAM_TYPE_OFFSET = 5
NUM_NODE_ID_OFFSET = 6
NODE_ID_OFFSET = 7

CAN_START_ARBITRATION_ID = 0b00000010000
CAN_ARBITRATION_ID = 0b00000000000


class DatagramTypeError(Exception):
    pass


class Datagram:
    """This class acts as an easy modular interface for a datagram."""

    def __init__(self, **kwargs):
        self._check_kwargs(**kwargs)

        self._protocol_version = kwargs["protocol_version"] & 0xff
        self._datagram_type_id = kwargs["datagram_type_id"] & 0xff

        self._node_ids = []
        for val in kwargs["node_ids"]:
            self._node_ids.append(val & 0xff)

        self._data = kwargs["data"]

    @classmethod
    def deserialize(cls, datagram_bytearray):
        """This function returns an instance of the class from a bytearray."""
        assert isinstance(datagram_bytearray, bytearray)

        # "theoretical" lower limit:
        # 1 (prot) + 4 (crc32) + 1 (type) + 1 (num nodes) + 0 (nodes) + 2 (data size) + 0 (data)
        #   = 9
        if len(datagram_bytearray) < MIN_BYTEARRAY_SIZE:
            raise DatagramTypeError(
                "Invalid Datagram format from bytearray: Does not meet minimum size requirement")

        protocol_version = datagram_bytearray[PROTOCOL_VERSION_OFFSET]
        crc32 = datagram_bytearray[CRC_32_OFFSET:DATAGRAM_TYPE_OFFSET]
        datagram_type_id = datagram_bytearray[DATAGRAM_TYPE_OFFSET]

        num_node_ids = datagram_bytearray[NUM_NODE_ID_OFFSET]

        if len(datagram_bytearray) < MIN_BYTEARRAY_SIZE + num_node_ids:
            raise DatagramTypeError("Invalid Datagram format from bytearray: Not enough node ids")

        node_ids = list(datagram_bytearray[NODE_ID_OFFSET:NODE_ID_OFFSET + num_node_ids])

        data_size = cls._convert_from_bytearray(cls,
                                                datagram_bytearray[NODE_ID_OFFSET + num_node_ids:], 2)

        if len(datagram_bytearray) != MIN_BYTEARRAY_SIZE + num_node_ids + data_size:
            raise DatagramTypeError("Invalid Datagram format from bytearray: Not enough data bytes")

        data = datagram_bytearray[NODE_ID_OFFSET + num_node_ids + DATA_SIZE_SIZE:]

        exp_crc32 = cls._calculate_crc32(cls, datagram_type_id, node_ids, data)

        if not exp_crc32 == crc32:
            raise DatagramTypeError("Invalid Datagram format from bytearray: Invalid crc32")

        return cls(protocol_version=protocol_version,
                   datagram_type_id=datagram_type_id, node_ids=node_ids, data=data)

    def serialize(self):
        """This function returns a bytearray based on set data."""

        node_crc32 = zlib.crc32(bytearray(self._node_ids))
        node_crc32 = self._convert_to_bytearray(node_crc32, 4)
        data_crc32 = zlib.crc32(self._data)
        data_crc32 = self._convert_to_bytearray(data_crc32, 4)

        crc32_array = bytearray([self._datagram_type_id,
                                 len(self._node_ids),
                                 *node_crc32,
                                 len(self._data) & 0xff,
                                 (len(self._data) >> 8) & 0xff,
                                 *data_crc32])
        # Update the crc32
        crc32 = zlib.crc32(crc32_array)
        crc32 = self._convert_to_bytearray(crc32, 4)

        # Update the bytearray
        return bytearray([self._protocol_version,
                          *crc32,
                          self._datagram_type_id,
                          len(self._node_ids),
                          *(self._node_ids),
                          len(self._data) & 0xff,
                          (len(self._data) >> 8) & 0xff,
                          *(self._data)])

    # Accessors and mutators for the datagram

    @property
    def protocol_version(self):
        """This function describes the protocol version property."""
        return self._protocol_version

    @protocol_version.setter
    def protocol_version(self, value):
        """This function sets the protocol version."""
        assert value & 0xff == value
        self._protocol_version = value & 0xff

    @property
    def datagram_type_id(self):
        """This function describes the datagram type id property."""
        return self._datagram_type_id

    @datagram_type_id.setter
    def datagram_type_id(self, value):
        """This function sets the datagram type id."""
        assert value & 0xff == value
        self._datagram_type_id = value & 0xff

    @property
    def node_ids(self):
        """This function describes the node ids property."""
        return self._node_ids

    @node_ids.setter
    def node_ids(self, value):
        """This function sets the node ids."""
        assert isinstance(value, list)
        for val in value:
            assert val & 0xff == val
            assert val < 64
        self._node_ids = value

    @property
    def data(self):
        return self._data

    @data.setter
    def data(self, value):
        """This function sets the data."""
        assert isinstance(value, bytearray)
        self._data = value

    def _check_kwargs(self, **kwargs):
        """ This function checks that all variables are as expected"""

        args = [
            "protocol_version",
            "datagram_type_id",
            "node_ids",
            "data"]
        values = ["protocol_version", "datagram_type_id"]

        # Check all arguments are present
        for arg in args:
            assert arg in kwargs

        # Check that types are as expected
        for value in values:
            assert not isinstance(kwargs[value], list)
        assert isinstance(kwargs["node_ids"], list)
        assert isinstance(kwargs["data"], bytearray)

        # Verify all inputs
        assert kwargs["protocol_version"] & 0xff == kwargs["protocol_version"]
        assert kwargs["datagram_type_id"] & 0xff == kwargs["datagram_type_id"]

    def _convert_to_bytearray(self, in_value, bytes):
        out_bytearray = bytearray()
        for i in range(0, bytes):
            out_bytearray.append((in_value >> (8 * i)) & 0xff)
        return out_bytearray

    def _convert_from_bytearray(self, in_bytearray, bytes):
        value = 0
        for i in range(0, bytes):
            value = value | ((in_bytearray[i] & 0xff) << (i * 8))
        return value

    def _calculate_crc32(self, datagram_type_id, node_ids, data):
        """This function returns a bytearray based on set data."""

        node_crc32 = zlib.crc32(bytearray(node_ids))
        node_crc32 = self._convert_to_bytearray(self, node_crc32, 4)
        data_crc32 = zlib.crc32(data)
        data_crc32 = self._convert_to_bytearray(self, data_crc32, 4)

        crc32_array = bytearray([datagram_type_id,
                                 len(node_ids),
                                 *node_crc32,
                                 len(data) & 0xff,
                                 (len(data) >> 8) & 0xff,
                                 *data_crc32])
        # Update the crc32
        crc32 = zlib.crc32(crc32_array)
        crc32 = self._convert_to_bytearray(self, crc32, 4)

        # Update the bytearray
        return crc32


class DatagramSender:
    """This class acts as a distributor for the Datagram class on a bus."""

    def __init__(self, bustype="socketcan", channel=DEFAULT_CHANNEL,
                 bitrate=CAN_BITRATE, receive_own_messages=False):
        print("Initializing CAN Bus...")
        self.bus = can.interface.Bus(
            bustype=bustype,
            channel=channel,
            bitrate=bitrate,
            receive_own_messages=receive_own_messages)

    def send(self, message):
        """This sends the Datagrams."""

        assert isinstance(message, Datagram)

        chunk_messages = self._chunkify(message.serialize(), 8)
        can_messages = []
        message_arbitration_id = CAN_START_ARBITRATION_ID
        message_extended_arbitration = False

        # Populate an array with the can message from the library
        for chunk_message in chunk_messages:
            can_messages.append(can.Message(arbitration_id=message_arbitration_id,
                                            data=chunk_message,
                                            is_extended_id=message_extended_arbitration))

            # After the first message, set the arbitration ID to 0
            message_arbitration_id = CAN_ARBITRATION_ID

        # Send the messages
        for msg in can_messages:
            self.bus.send(msg)
        print("{} messages were sent on {}".format(len(can_messages), self.bus.channel_info))

    def _chunkify(self, data, size):
        """This chunks up the datagram bytearray for easy iteration."""
        return (data[pos:pos + size] for pos in range(0, len(data), size))


class DatagramListener(can.BufferedReader):
    """This class handles a callback when a datagram is received."""

    def __init__(self, callback):
        """This registers the callback."""
        assert callable(callback)
        self.callback = callback
        # Messages are stored in a dictionary where key = board ID, value = message
        self.datagram_messages = {}

        super().__init__()

    def on_message_received(self, msg: can.Message):
        """This (SHOULD) wait for the first message in the datagram 0x010."""
        super().on_message_received(msg)

        board_id = (msg.arbitration_id & 0b11111100000) >> 5
        start_message = (msg.arbitration_id & 0x10) >> 4

        if start_message == 1:
            # Every time we receive the first message, we reset the variables.
            self.datagram_messages[board_id] = msg.data

        if start_message == 0:
            self.datagram_messages[board_id] += msg.data

        try:
            datagram = Datagram.deserialize(self.datagram_messages[board_id])
        except DatagramTypeError:
            # Datagram is incomplete, continue until complete
            pass
        else:
            # Datagram is complete, call the callback with formed datagram
            self.callback(datagram)
