# pylint: skip-file
"""This client script handles datagram protocol communication between devices on the CAN."""

import zlib
import can

DEFAULT_CHANNEL = "can0"
PROT_VER = 1
CAN_BITRATE = 500000

MESSAGE_SIZE = 8
HEADER_SIZE = 6


class Datagram:
    """This class acts as an easy modular interface for a datagram."""

    def __init__(self, **kwargs):
        self._protocol_version = 0
        self._datagram_type_id = 0
        self._node_ids = []
        self._data = bytearray(0)

        self._check_kwargs(**kwargs)

        self._protocol_version = kwargs["protocol_version"] & 0xff
        self._datagram_type_id = kwargs["datagram_type_id"] & 0xff

        self._node_ids = []
        for val in kwargs["node_ids"]:
            self._node_ids.append(val & 0xff)

        self._data = kwargs["data"]

    def deserialize(self, datagram_bytearray):
        """This function sets the bytearray and datagram from the bytearray."""
        assert isinstance(datagram_bytearray, bytearray)

        # "theoretical" lower limit:
        # 1 (prot) + 4 (crc32) + 1 (type) + 1 (num nodes) + 1 (nodes) + 2 (data size) + 1 (data)
        #   = 11
        assert len(datagram_bytearray) > 11
        protocol_version = datagram_bytearray[0]
        datagram_type_id = datagram_bytearray[5]

        num_node_ids = datagram_bytearray[6]
        assert len(datagram_bytearray) > 7 + num_node_ids
        node_ids = []
        i = 0
        while i < num_node_ids:
            node_ids.append(datagram_bytearray[7 + i])
            i += 1

        assert len(datagram_bytearray) > 7 + num_node_ids
        data_size = (datagram_bytearray[7 + num_node_ids] &
                     0xff) << 8 | datagram_bytearray[7 + num_node_ids + 1] & 0xff

        assert len(datagram_bytearray) == 7 + num_node_ids + 2 + data_size
        data = []
        i = 0
        while i < data_size:
            data.append(datagram_bytearray[7 + num_node_ids + 2 + i])
            i += 1

        self._protocol_version = protocol_version
        self._datagram_type_id = datagram_type_id
        self._node_ids = node_ids
        self._data = bytearray(data)

    def serialize(self):
        """This function updates the bytearray based on data."""

        node_crc32 = zlib.crc32(bytearray(self._node_ids))
        node_crc32 = bytearray([node_crc32 & 0xff, (node_crc32 >> (8 * 1)) & 0xff,
                                (node_crc32 >> (8 * 2)) & 0xff, (node_crc32 >> (8 * 3)) & 0xff])
        data_crc32 = zlib.crc32(self._data)
        data_crc32 = bytearray([data_crc32 & 0xff, (data_crc32 >> (8 * 1)) & 0xff,
                                (data_crc32 >> (8 * 2)) & 0xff, (data_crc32 >> (8 * 3)) & 0xff])

        crc32_array = bytearray([self._datagram_type_id,
                                 len(self._node_ids),
                                 *node_crc32,
                                 len(self._data) & 0xff,
                                 (len(self._data) >> 8) & 0xff,
                                 *data_crc32])
        # Update the crc32
        crc32 = zlib.crc32(crc32_array)
        crc32 = bytearray([crc32 & 0xff, (crc32 >> (8 * 1)) & 0xff,
                           (crc32 >> (8 * 2)) & 0xff, (crc32 >> (8 * 3)) & 0xff])

        # Update the bytearray
        return bytearray([self._protocol_version,
                          *crc32,
                          self._datagram_type_id,
                          len(self._node_ids),
                          *(self._node_ids),
                          (len(self._data) >> 8) & 0xff,
                          len(self._data) & 0xff,
                          *(self._data)])

    # Accessors and mutators for the datagram
    def get_protocol_version(self):
        """This function retrieves the protocol version."""
        return self._protocol_version

    def set_protocol_version(self, protocol_version):
        """This function sets the protocol version."""
        assert protocol_version & 0xff == protocol_version
        self._protocol_version = protocol_version & 0xff

    def get_datagram_type_id(self):
        """This function retrieves the type id."""
        return self._datagram_type_id

    def set_datagram_type_id(self, datagram_type_id):
        """This function sets the type id."""
        assert datagram_type_id & 0xff == datagram_type_id
        self._datagram_type_id = datagram_type_id & 0xff

    def get_node_ids(self):
        """This function sets the node ids."""
        return self._node_ids

    def set_node_ids(self, node_ids):
        """This function sets the node ids."""
        assert isinstance(node_ids, list)
        for val in node_ids:
            assert val & 0xff == val
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
        assert kwargs["protocol_version"] & 0xff == kwargs["protocol_version"]
        assert kwargs["datagram_type_id"] & 0xff == kwargs["datagram_type_id"]


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

    def get_bus(self):
        """Accessor for the bus"""
        return self.bus

    def _chunkify(self, data, size):
        """This chunks up the datagram bytearray for easy iteration."""
        return (data[pos:pos + size] for pos in range(0, len(data), size))


class DatagramListener(can.BufferedReader):
    """This class handles a callback when a datagram is received."""

    def __init__(self, callback):
        """This registers the callback."""
        assert callable(callback)
        self.callback = callback
        self.datagram_messages = []
        self.receiving_datagram = False
        self.num_node_id = -1
        self.incomplete_data_bytes = False
        self.num_data_bytes = -1

        super().__init__()

    def on_message_received(self, msg):
        """This (SHOULD) wait for the first message in the datagram 0x010."""
        super().on_message_received(msg)

        if(msg.arbitration_id == 0x0010 and not self.receiving_datagram):
            # Every time we receive the first message, we reset the variables.
            self.datagram_messages = []
            self.receiving_datagram = False
            self.num_node_id = -1
            self.incomplete_data_bytes = False
            self.num_data_bytes = -1

            # Here we want to handle what happens when we get the first message in a datagram
            first_bytearray = msg.data

            # Get the number of node ids
            self.num_node_id = first_bytearray[HEADER_SIZE]

            # Determine if there are node ids in this message.
            # If ALL node ids are in this message, determine if the data size is in this message
            bytes_remaining = MESSAGE_SIZE - (HEADER_SIZE + 1)
            if self.num_node_id < bytes_remaining:
                bytes_remaining = bytes_remaining - self.num_node_id
                if bytes_remaining == 1:
                    self.num_data_bytes = first_bytearray[HEADER_SIZE + self.num_node_id + 1] << 8
                    self.incomplete_data_bytes = True
                elif bytes_remaining >= 2:
                    upper_bits = first_bytearray[HEADER_SIZE + self.num_node_id + 1] << 8
                    lower_bits = first_bytearray[HEADER_SIZE + self.num_node_id + 2]
                    self.num_data_bytes = upper_bits | lower_bits
                    self.incomplete_data_bytes = False

                    if bytes_remaining > 2:
                        # Determine the index of the first data byte (If this is 8, then it is in
                        # the next message)
                        first_data_index = HEADER_SIZE + self.num_node_id

                        # If there are data bytes in this message, subtract that number and continue
                        if first_data_index < MESSAGE_SIZE:
                            self.num_data_bytes = self.num_data_bytes - \
                                (MESSAGE_SIZE - (first_data_index))
                        # Otherwise, we continue to the next message
            self.num_node_id = self.num_node_id - bytes_remaining

            # Add the data to the array, and continue receiving messages
            for byte in msg.data:
                self.datagram_messages.append(byte)
            self.receiving_datagram = True

        elif msg.arbitration_id == 0x0000 and self.receiving_datagram:
            # Here we want to handle what happens when we get the next message in a datagram
            message_bytearray = msg.data

            # We handle if there are still node ids in this message
            if self.num_node_id > 0:
                # If there are more node ids than the message size, continue on to the next message
                # Otherwise, locate the index of the data size

                if self.num_node_id >= MESSAGE_SIZE:
                    self.num_node_id = self.num_node_id - MESSAGE_SIZE
                else:
                    bytes_remaining = MESSAGE_SIZE - self.num_node_id
                    if bytes_remaining == 1:
                        self.num_data_bytes = message_bytearray[self.num_node_id] << 8
                        self.incomplete_data_bytes = True
                    elif bytes_remaining >= 2:
                        self.num_data_bytes = message_bytearray[self.num_node_id] << 8 | message_bytearray[self.num_node_id + 1]
                        self.incomplete_data_bytes = False

                        if bytes_remaining > 2:
                            # Determine the index of the first data byte (If this is 8, then it is
                            # in the next message)
                            first_data_index = self.num_node_id

                            # If there are data bytes in this message, subtract that number and
                            # continue
                            if first_data_index < MESSAGE_SIZE:
                                self.num_data_bytes = self.num_data_bytes - \
                                    (MESSAGE_SIZE - (first_data_index))
                            # Otherwise, we continue to the next message
                    self.num_node_id = 0
            else:
                # There are no more node ids.
                # If we only got half of the data bytes
                bytes_remaining = MESSAGE_SIZE
                if self.incomplete_data_bytes:
                    self.num_data_bytes = self.num_data_bytes | message_bytearray[0]
                    bytes_remaining -= 1
                # If we already have the data bytes, use how many is remaining (otherwise
                # it was just set)
                if self.num_data_bytes > bytes_remaining:
                    self.num_data_bytes = self.num_data_bytes - bytes_remaining
                else:
                    # There are no more data bytes left, message is complete!
                    self.receiving_datagram = False
            for byte in msg.data:
                self.datagram_messages.append(byte)

            # Once we're on the last message, call the callback
            if not self.receiving_datagram:
                self.callback(self.datagram_messages)
                self.datagram_messages = []
                self.receiving_datagram = False
                self.num_node_id = -1
                self.incomplete_data_bytes = False
                self.num_data_bytes = -1
        else:
            # If the message gets interrupted, or regardless when theres a non datagram message,
            #   just reset everything.
            self.datagram_messages = []
            self.receiving_datagram = False
            self.num_node_id = -1
            self.incomplete_data_bytes = False
            self.num_data_bytes = -1
