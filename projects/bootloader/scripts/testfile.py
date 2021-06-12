# pylint: skip-file
import can_datagram
import can

test_nodes = [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
test_data = [3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5, 8, 9, 7, 9, 3, 2, 3, 8, 4, 6, 2, 6, 4, 3, 3]

# Create the Message
message = can_datagram.Datagram(
    protocol_version=1,
    datagram_type_id=2,
    node_ids=test_nodes,
    data=bytearray(test_data))

assert message.get_protocol_version() == 1
assert message.get_datagram_type_id() == 2
assert message.get_node_ids()[1] == 2
assert message.get_data()[1] == 1

# Create the Sender
sender = can_datagram.DatagramSender(bustype="socketcan", channel="vcan0")
bus = sender.bus

# Create a Listener


def StandardCallback(msgs):
    print("Hey, you got a message! Here it is:")
    for msg in msgs:
        print(*msg)


listener = can_datagram.DatagramListener(StandardCallback)

# Register the bus and listener
notifier = can.Notifier(bus, [listener])

# Send a message
sender.send(message)

sender.send(message)

sender.send(message)

message.deserialize(bytearray(
    b'\x01\x00\x00\x00\t\x02\n\x01\x02\x03\x04\x05\x06\x07\x08\t\n\x01\n\x03\x01\x04\x01\x05\t\x02\x06\x05\x03\x05\x08\t\x07\t\x03\x02\x03\x08\x04\x06\x02\x06\x04\x03\x03'))
print(message.get_data())

while(True):
    pass
