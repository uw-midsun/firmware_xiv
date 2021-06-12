# pylint: skip-file
"""This client script handles datagram protocol communication between devices on the CAN."""

import zlib
import can
import time

DEFAULT_CHANNEL = "can0"
PROT_VER = 1
CAN_BITRATE = 500000


class Datagram:
    """This class acts as an easy modular interface for a datagram."""

    def __init__(self, **kwargs):
        self._protocol_version = 0
        self._datagram_type_id = 0
        self._node_ids = []
        self._data = bytearray(0)
        self._datagram_bytearray = bytearray(0)

        self._check_kwargs(**kwargs)

        self._protocol_version = kwargs["protocol_version"] & 0xf
        self._datagram_type_id = kwargs["datagram_type_id"] & 0xf

        self._node_ids = []
        for val in kwargs["node_ids"]:
            self._node_ids.append(val & 0xf)

        self._data = kwargs["data"]

    def deserialize(self, datagram_bytearray):
        """This function sets the bytearray and datagram from the bytearray."""
        assert isinstance(datagram_bytearray, bytearray)

        # "theoretical" lower limit
        assert len(datagram_bytearray) > 9
        protocol_version = datagram_bytearray[0]
        datagram_type_id = datagram_bytearray[1]
        # crc32 = datagram_bytearray[2] << 3 + datagram_bytearray[3] << 2 + \
        #     datagram_bytearray[4] << 1 + datagram_bytearray[5]

        num_node_ids = datagram_bytearray[6]
        assert len(datagram_bytearray) > 7 + num_node_ids
        node_ids = []
        i = 0
        while i < num_node_ids:
            node_ids.append(datagram_bytearray[6 + i])
            i += 1

        data_size = datagram_bytearray[6 + num_node_ids]
        assert len(datagram_bytearray) == 7 + num_node_ids + 1 + data_size
        data = []
        while i < data_size:
            data.append(datagram_bytearray[6 + i])
            i += 1

        self._protocol_version = protocol_version
        self._datagram_type_id = datagram_type_id
        self._node_ids = node_ids
        self._data = data
        self._datagram_bytearray = datagram_bytearray

    def serialize(self):
        """This function updates the bytearray based on data."""
        crc32_array = bytearray([self._protocol_version,
                                 self._datagram_type_id,
                                 len(self._node_ids),
                                 *(self._node_ids),
                                 len(self._data) & 0xf0,
                                 len(self._data) & 0x0f,
                                 *(self._data)])

        # Update the crc32
        crc32 = zlib.crc32(crc32_array) & 0xf

        # Update the bytearray
        return bytearray([self._protocol_version,
                          crc32,
                          self._datagram_type_id,
                          len(self._node_ids),
                          *(self._node_ids),
                          len(self._data) & 0xf0,
                          len(self._data) & 0x0f,
                          *(self._data)])

    # Accessors and mutators for the datagram
    def get_protocol_version(self):
        """This function retrieves the protocol version."""
        return self._protocol_version

    def set_protocol_version(self, protocol_version):
        """This function sets the protocol version."""
        assert protocol_version & 0xf == protocol_version
        self._protocol_version = protocol_version & 0xf

    def get_datagram_type_id(self):
        """This function retrieves the type id."""
        return self._datagram_type_id

    def set_datagram_type_id(self, datagram_type_id):
        """This function sets the type id."""
        assert datagram_type_id & 0xf == datagram_type_id
        self._datagram_type_id = datagram_type_id & 0xf

    def get_node_ids(self):
        """This function sets the node ids."""
        return self._node_ids

    def set_node_ids(self, node_ids):
        """This function sets the node ids."""
        assert isinstance(node_ids, list)
        for val in node_ids:
            assert val & 0xf == val
            assert val < 64
        self._node_ids = node_ids

    def get_data(self):
        """This function retrieves the data."""
        return self._data

    def set_data(self, data):
        """This function sets the data."""
        assert isinstance(data, bytearray)
        self._data = data

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
        assert kwargs["protocol_version"] & 0xf == kwargs["protocol_version"]
        assert kwargs["datagram_type_id"] & 0xf == kwargs["datagram_type_id"]


class DatagramSender:
    def __init__(self, bustype="socketcan", channel=DEFAULT_CHANNEL, bitrate=CAN_BITRATE):
        print("Initializing CAN Bus...")
        self.bus = can.interface.Bus(bustype=bustype, channel=channel, bitrate=bitrate)

    def send(self, message):
        """This sends the Datagrams."""

        assert isinstance(message, Datagram)

        chunk_messages = self._chunkify(message.serialize(), 8)
        can_messages = []
        message_arbitration_id = 0b00000010000
        message_extended_arbitration = False

        # Populate an array with the can message from the library
        for chunk_message in chunk_messages:
            can_messages.append(can.Message(arbitration_id=message_arbitration_id,
                                            data=chunk_message,
                                            is_extended_id=message_extended_arbitration))

            # After the first message, set the arbitration ID to 0
            message_arbitration_id = 0b00000000000

        # Send the messages
        try:
            for msg in can_messages:
                self.bus.send(msg)
            print("{} messages were sent on {}".format(len(can_messages), self.bus.channel_info))

        except can.CanError:
            print("Message could not be sent")

    def _chunkify(self, data, size):
        """This chunks up the datagram bytearray for easy iteration."""
        return (data[pos:pos + size] for pos in range(0, len(data), size))


class DatagramListener(can.BufferedReader):
    def __init__(self, callback):
        """This registers the callback."""
        assert callable(callback)
        self.callback = callback
        self.datagram_messages = []
        self.receiving_datagram = False

        super().__init__()

    def on_message_received(self, msg):
        """This (SHOULD) wait for the first message in the datagram 0x010, and gets the whole datagram."""
        super().on_message_received(msg)

        print("=====MESSAGE=====", msg)
        if(msg.arbitration_id == 0x0010 and not self.receiving_datagram):
            print("CASE 1 ")
            self.datagram_messages.append(msg.data)
            self.receiving_datagram = True
        elif(msg.arbitration_id == 0x0000 and self.receiving_datagram):
            print("CASE 2 ")
            self.datagram_messages.append(msg.data)
            self.receiving_datagram = True
        elif(msg.arbitration_id != 0x0000 and self.receiving_datagram):
            print("CASE 3 ")
            self.callback(self.datagram_messages)
            self.datagram_messages = []
            self.receiving_datagram = False
