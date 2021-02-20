#!/usr/bin/env python3
""" Dumps CAN information from SocketCAN/serial """
import argparse
from abc import abstractmethod
import datetime
import logging
import os
import socket
import struct
import serial
import serial.tools.list_ports
from cobs import cobs

from can_message import CanMessage


def select_device():
    """User-provided serial device selector.

    Args:
        None

    Returns:
        The selected serial device as ListPortInfo.
    """
    while True:
        print('Pick the serial device:')
        ports = serial.tools.list_ports.comports()
        for i, port in enumerate(ports):
            print('{}: {}'.format(i, port))

        try:
            chosen_port = ports[int(input())]
            print('Selected {}'.format(chosen_port))
            return chosen_port
        except IndexError:
            print('Invalid device!')
            continue


class CanDataSource:
    """An abstract class representing a CAN data source.

    A CAN data source is an interface that can be listened on for a CAN message
    representation.
    """

    def __init__(self, masked=None):
        self.masked = masked or []

    @abstractmethod
    def get_packet(self):
        """Fetch the next packet from the CAN data source.

        Args:
            None

        Returns:
            A tuple containing the system CAN bus message ID and the message
            data.
        """

    def run(self):
        """Start reading from the CAN data source.

        This reads messages from the data source, parses each one and prints
        to stdout, before logging into a log file until the process terminates.

        Args:
            None

        Returns:
            Does not return.
        """
        while True:
            # CAN ID, data, DLC
            can_id, data = self.get_packet()
            if can_id not in self.masked:
                can_msg = CanMessage(can_id, data)
                can_msg.parse()

            # pylint: disable=logging-format-interpolation
            logging.info('{},{},{}'.format(can_id, data, len(data)))


class SocketCanDataSource(CanDataSource):
    """
    A CAN data source that reads from a bound SocketCAN interface (slcan0, can0, etc.)
    """
    CAN_FRAME_FMT = "<IB3x8s"

    def __init__(self, masked, device):
        super().__init__(masked)
        self.sock = socket.socket(socket.PF_CAN, socket.SOCK_RAW, socket.CAN_RAW)
        self.sock.bind((device,))

    def get_packet(self):
        can_pkt = self.sock.recv(16)
        can_id, length, data = struct.unpack(self.CAN_FRAME_FMT, can_pkt)
        can_id &= socket.CAN_EFF_MASK
        data = data[:length]

        return can_id, data


class SerialCanDataSource(CanDataSource):
    """
    A CAN datasource that reads from a bound serial port interface.
    """

    def __init__(self, masked, device):
        super().__init__(masked)
        self.ser = serial.Serial(device, 115200)

    def get_packet(self):
        while True:
            encoded_line = self.readline()
            try:
                line = cobs.decode(encoded_line[:-1])
            except cobs.DecodeError:
                # print('COBS decode error (len {})'.format(len(encoded_line)))
                continue

            if len(line) != 16:
                # print('Invalid line (len {})'.format(len(line)))
                continue

            header = int.from_bytes(line[0:4], 'little')
            dlc = header >> 28 & 0xF

            can_id = int.from_bytes(line[4:8], 'little') & 0x7FF
            data = line[8:8 + dlc]

            return can_id, data

    def readline(self, eol=b'\x00'):
        """Readline with arbitrary EOL delimiter

        Args:
            ser: Serial device to read
            eol: Bytes to use a EOL delimiter

        Returns:
            All data read from the device until the EOL delimiter was found.
        """
        leneol = len(eol)
        line = bytearray()
        while True:
            read_char = self.ser.read(1)
            if read_char:
                line += read_char
                if line[-leneol:] == eol:
                    break
            else:
                break
        return bytes(line)


def main():
    """Main entry point"""
    parser = argparse.ArgumentParser()
    parser.add_argument('-l', '--log_dir', help='Directory for storing logs',
                        nargs='?', default='logs')
    parser.add_argument('-m', '--mask', help='Mask message ID from being parsed', action='append')
    parser.add_argument('device', help='Serial device or "slcan0" or "can0"')
    args = parser.parse_args()

    date_formatted = datetime.datetime.now().strftime('%Y-%m-%d %H.%M.%S.%f')
    log_file = '{}/system_can_{}.log'.format(args.log_dir, date_formatted)
    os.makedirs(os.path.dirname(log_file), exist_ok=True)
    logging.basicConfig(level=logging.DEBUG, format='"%(asctime)s",%(message)s', filename=log_file)

    print('Masking IDs {}'.format(args.mask))
    if args.device in ['slcan0', 'can0']:
        datasource = SocketCanDataSource(masked=args.mask, device=args.device)
    else:
        datasource = SerialCanDataSource(masked=args.mask, device=args.device)

    datasource.run()


if __name__ == '__main__':
    main()
