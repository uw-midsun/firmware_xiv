"""A can.Bus implementation for the CAN-over-serial protocol from can_uart.h."""

import can
from cobs import cobs
import serial
import struct

# Message format, in struct syntax:
# <: Little-endian marker
# 3s: CTX or CRX
# B: Header byte - extended flag, dlc
# I: u32 ID
# Q: u64 data
MSG_FORMAT = '<3sBI8s'

class SerialBus(can.BusABC):

    def __init__(self, channel, baudrate=115200, timeout=0.1, rtscts=False,
                *args, **kwargs):
        self.channel_info = "CAN-UART serial protocol: " + channel
        self.serial = serial.serial_for_url(
            channel, baudrate=baudrate, timeout=timeout, rtscts=rtscts)
        super(SerialBus, self).__init__(channel=channel, *args, **kwargs)

    def send(self, msg: can.Message, timeout=None):
        header_byte = (
            (int(msg.is_extended_id) & 0x1) |
            (int(msg.is_remote_frame) & 0x1) << 1 |
            (int(msg.dlc) & 0xF) << 4)
        try:
            serialized = struct.pack(MSG_FORMAT,
                b'CRX', header_byte, msg.arbitration_id, msg.data)
        except struct.error:
            raise ValueError('Could not encode message')
        print('writing', cobs.encode(serialized) + b'\0')
        self.serial.write(cobs.encode(serialized) + b'\0')  # final null separator

    def _recv_internal(self, timeout):
        self.serial.timeout = timeout
        try:
            rx = self.serial.read_until(b'\0')
        except serial.SerialException:
            return None, False

        if not rx:
            return None, False

        print('RECEIVED:', rx)
        if b'CTX' not in rx[1:] and b'\0' not in rx:
            return None, False

        rx = rx[rx[1:].find(b'CTX'):rx.rfind(b'\0')]
        print('DECODING:', rx)

        try:
            deserialized = struct.unpack(MSG_FORMAT, cobs.decode(rx))
        except (struct.error, cobs.DecodeError) as e:
            print(e)
            return None, False

        marker = deserialized[0].decode('ascii')
        header = deserialized[1]
        extended = bool(header & 0x1)
        rtr = bool((header >> 1) & 0x1)
        dlc = (header >> 4) & 0xF
        arbitration_id = deserialized[2]
        data = deserialized[3]

        print(marker, extended, rtr, dlc, hex(arbitration_id), data)

        msg = can.Message(arbitration_id=arbitration_id, extended_id=extended, is_remote_frame=rtr,
            dlc=dlc, data=data)
        return msg, False
