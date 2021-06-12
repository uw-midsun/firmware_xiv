# pylint: skip-file
"""This client script handles datagram protocol communication between devices on the CAN."""

import sys
import asyncio
import can

default_channel = "can0"  # pylint: disable=invalid-name
prot_ver = 1

# We use the standard 500kbps baudrate.
CAN_BITRATE = 500000


class CanDatagram:
    """This class implements the Datagram modules."""

    def __init__(self, channel=default_channel):
        print("Initializing CAN Bus...")

        self.bus = can.interface.Bus(bustype="socketcan", channel=channel, bitrate=CAN_BITRATE)

        self.reader = can.Listener()
        self.notifier = can.AsyncBufferedReader(self.bus, [self.reader])

        # Instead, try to make a new listener for each callback
        # Also note, that the callbacks are called for DATAGRAMS not messages
        self.callbacks = []

        # Starts an infinite loop that keeps checking for messages
        asyncio.get_event_loop().create_task(self.init_callback_listener())

    def send_datagram_msg(self, prot_ver, crc32, type_id, num_node_ids, node_ids, data_size, data):
        """This class sends the Datagrams."""
        # Assert input data is of the right size
        self.__check_datagram_msg_size(
            prot_ver,
            crc32,
            type_id,
            num_node_ids,
            node_ids,
            data_size,
            data)

        # STRATEGY: At this point, build up the ENTIRE array of data, where each
        # element is 1 byte and send them in 8-sized chunks

        # Set the start message
        message_arbitration_id = 0b00000010000
        message_extended_arbitration = False

        # NOTE: CRC32 needs to be split, check the message data
        start_message_data = [prot_ver, crc32, type_id, num_node_ids, node_ids]

        start_message = can.Message(arbitration_id=message_arbitration_id,
                                    data=start_message_data,
                                    is_extended_id=message_extended_arbitration)

        # Prepare the rest of the messages
        message_arbitration_id = 0b00000000000
        num_messages = 0
        messages = []

        # treat the data as a bytearray (look into bytearray)
        while data != 0:
            # Grab the first 8 bytes of the message
            message_data_part = data & 0xFF
            data = data >> 8
            num_messages += 1

            messages[num_messages] = can.Message(arbitration_id=message_arbitration_id,
                                                 data=message_data_part,
                                                 is_extended_id=message_extended_arbitration)

        # Send the messages
        try:
            self.bus.send(start_message)
            print("Start message was sent on {}".format(self.bus.channel_info))

            for msg in messages:
                self.bus.send(msg)
            print("{} messages were sent on {}".format(num_messages + 1, self.bus.channel_info))

        except can.CanError:
            print("Message could not be sent")

    def print_msgs(self):
        """This prints all messages on the bus"""

        # Note: not sure how this works, might block forever?
        for msg in self.bus:
            print(msg.data)

    def add_recv_callback(self, callback):
        """This registers a callback that is called whenever a message is received"""
        self.callbacks.append(callback)

    # Utility functions to help the rest of the class

    async def init_callback_listener(self):
        """This infinite loop waits for messages to come on the BUS, and then calls the callback."""
        # Try to add this event loop in another thread
        while True:
            msg = await self.notifier.get_message()
            if msg is not None:
                for callback in self.callbacks:
                    callback(msg)

    def __check_datagram_msg_size(self, prot_ver, crc32, type_id,
                                  num_node_ids, node_ids, data_size, data):
        """This is a utility class to assert the size of the variables."""
        assert sys.getsizeof(prot_ver) == 1
        assert sys.getsizeof(crc32) == 4
        assert sys.getsizeof(type_id) == 1
        assert sys.getsizeof(num_node_ids) == 1
        assert sys.getsizeof(node_ids) == 1
        assert sys.getsizeof(data_size) == 2
        assert sys.getsizeof(data) == data_size
