import can_datagram

can = CanDatagram()

# pylint: skip-file
# # USE vcan0 instead of can0

prot_ver = 0x00

crc32 = 0x00000000
type_id = 0x00
num_node_ids = 0x00
node_ids = 0x00
data_size = 0x0003
data = 0x112233


def test_callback(message):
    print(message)


can.add_recv_callback(test_callback)

can.send_datagram_msg(prot_ver, crc32, type_id, num_node_ids, node_ids, data_size, data)

# USE THE PYTHON UNIT TEST MODULE

# KEY: LOOPBACK MODE FOR TESTING

# Test sending
# 1. Send the datagram
# 2. Listen to the CAN channel
# 3. Get the bytes back to check -> register it to listen to the message
# sent (create new listeners within the test)

# Test receiving
# 1. Create CAN messages, call receive method repeatedly for all
# datagrams, compare received versus expected
