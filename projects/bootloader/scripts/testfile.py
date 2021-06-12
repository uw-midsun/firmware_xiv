# pylint: skip-file
import can_datagram

test_data = [3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5, 8, 9, 7, 9, 3, 2, 3, 8, 4, 6, 2, 6, 4, 3, 3]

message = can_datagram.Datagram(
    protocol_version=1,
    datagram_type_id=2,
    node_ids=[
        1,
        1],
    data=bytearray(test_data))

assert message.get_protocol_version() == 1
assert message.get_datagram_type_id() == 2
assert message.get_node_ids()[1] == 1
assert message.get_data()[1] == 4
assert message.get_bytearray() == bytearray(b'\x01\x06\x02\x02\x01\x01\x00\x02\r\x04')

message.set_protocol_version(2)
assert message.get_protocol_version() == 2
assert message.get_bytearray() == bytearray(b'\x02\x03\x02\x02\x01\x01\x00\x02\r\x04')

message.set_datagram_type_id(3)
assert message.get_datagram_type_id() == 3
assert message.get_bytearray() == bytearray(b'\x02\r\x03\x02\x01\x01\x00\x02\r\x04')

message.set_data(30, 1534263453221223332)
assert message.get_data_size() == 30
assert message.get_data() == [4, 10, 11, 7, 13, 1, 9, 5, 11, 7, 12, 12, 10, 4, 5, 1]

total_data = 0
for byte in message.get_data():
    total_data << 4
    total_data = total_data | byte
print(total_data)
