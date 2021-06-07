# pylint: skip-file
"""This client script handles datagram protocol communication between devices on the CAN."""

import zlib
# import can

DEFAULT_CHANNEL = "can0"  # pylint: disable=invalid-name
PROT_VER = 1

# We use the standard 500kbps baudrate.
CAN_BITRATE = 500000

# class Datagram_Listener:
#     def __init__(self):
#         return
#
# class Datagram_Sender:
#     def __init__(self):
#         return


class DatagramMessage:
    """This class acts as an easy modular interface for a datagram."""

    def __init__(self, **kwargs):
        self._protocol_version = 0
        self._datagram_type_id = 0
        self._crc32 = 0
        self._node_ids = []
        self._data = 0
        self._datagram_bytearray = bytearray(0)

        if "datagram_bytearray" in kwargs:
            self.set_data_from_bytearray(kwargs["datagram_bytearray"])
        else:
            self._check_kwargs(**kwargs)
            self.set_data_from_datagram(**kwargs)

    # Mutators for the bytearray and datagram representation
    def set_data_from_datagram(self, **kwargs):
        """This function sets the bytearray and datagram from arguments."""
        self._protocol_version = kwargs["protocol_version"] & 0xf
        self._datagram_type_id = kwargs["datagram_type_id"] & 0xf

        self._node_ids = []
        for val in kwargs["node_ids"]:
            self._node_ids.append(val & 0xf)

        self._data = []
        data = kwargs["data"]
        while data != 0:
            self._data.append(data & 0xf)
            data = data >> 4

        self._update_bytearray()

    def set_data_from_bytearray(self, datagram_bytearray):
        """This function sets the bytearray and datagram from the bytearray."""
        assert isinstance(datagram_bytearray, bytearray)

        # "theoretical" lower limit
        assert len(datagram_bytearray) > 9
        protocol_version = datagram_bytearray[0]
        datagram_type_id = datagram_bytearray[1]
        crc32 = datagram_bytearray[2] << 3 + datagram_bytearray[3] << 2 + \
            datagram_bytearray[4] << 1 + datagram_bytearray[5]

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
        self._crc32 = crc32
        self._node_ids = node_ids
        self._data = data
        self._datagram_bytearray = datagram_bytearray

    # Accessor for the bytearray
    def get_bytearray(self):
        """This function retrieves the bytearray."""
        return self._datagram_bytearray

    # Accessors and mutators for the datagram
    def get_protocol_version(self):
        """This function retrieves the protocol version."""
        return self._protocol_version

    def set_protocol_version(self, protocol_version):
        """This function sets the protocol version."""
        assert protocol_version & 0xf == protocol_version
        self._protocol_version = protocol_version & 0xf
        self._update_bytearray()

    def get_datagram_type_id(self):
        """This function retrieves the type id."""
        return self._datagram_type_id

    def set_datagram_type_id(self, datagram_type_id):
        """This function sets the type id."""
        assert datagram_type_id & 0xf == datagram_type_id
        self._datagram_type_id = datagram_type_id & 0xf
        self._update_bytearray()

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
        self._update_bytearray()

    def get_data(self):
        """This function retrieves the data."""
        return self._data

    def set_data(self, data):
        """This function sets the data."""
        assert isinstance(data, bytearray)
        self._data = data

    # Utility function to update the bytearray when the datagram is changed
    def _update_bytearray(self):
        """This function updates the bytearray based on data."""
        crc32_array = bytearray([self._protocol_version,
                                 self._datagram_type_id,
                                 len(self._node_ids),
                                 *(self._node_ids),
                                 len(self._data) & 0xf0,
                                 len(self._data) & 0x0f,
                                 *(self._data)])

        # Update the crc32
        self._crc32 = zlib.crc32(crc32_array) & 0xf

        # Update the bytearray
        self._datagram_bytearray = bytearray([self._protocol_version,
                                              self._crc32,
                                              self._datagram_type_id,
                                              len(self._node_ids),
                                              *(self._node_ids),
                                              len(self._data) & 0xf0,
                                              len(self._data) & 0x0f,
                                              *(self._data)])

    def _check_kwargs(self, **kwargs):
        """ This function checks that all variables are as expected"""

        args = [
            "protocol_version",
            "datagram_type_id",
            "node_ids",
            "data"]
        values = ["protocol_version", "datagram_type_id", "data"]
        arrays = ["node_ids"]

        # Check all arguments are present
        for arg in args:
            assert arg in kwargs

        # Check that types are as expected
        for value in values:
            assert not isinstance(kwargs[value], list)
        for array in arrays:
            assert isinstance(kwargs[array], list)

        # Verify all inputs
        assert kwargs["protocol_version"] & 0xf == kwargs["protocol_version"]
        assert kwargs["datagram_type_id"] & 0xf == kwargs["datagram_type_id"]
