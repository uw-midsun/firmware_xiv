# pylint: skip-file
import can_datagram
import can

test_data = [3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5, 8, 9, 7, 9, 3, 2, 3, 8, 4, 6, 2, 6, 4, 3, 3]

# Create the Message
message = can_datagram.Datagram(
    protocol_version=1, datagram_type_id=2, node_ids=[
        1, 1], data=bytearray(test_data))

assert message.get_protocol_version() == 1
assert message.get_datagram_type_id() == 2
assert message.get_node_ids()[1] == 1
assert message.get_data()[1] == 1

# Create the Sender
sender = can_datagram.DatagramSender(bustype="socketcan", channel="vcan0")
bus = sender.bus

# Create a Listener


def StandardCallback(msg):
    print("Hey, you got a message! Here it is:")
    print(msg)


listener = can_datagram.DatagramListener(StandardCallback)

# Register the bus and listener
notifier = can.Notifier(bus, [listener])

# Send a message
sender.send(message)

while(True):
    pass
