"""Parses CAN logs created by the system dump tool
"""
import argparse
import csv

from can_message import CanMessage


def parse_data(file_name):
    """Read the CSV data and parse it"""
    log_reader = csv.reader(open(file_name, 'r'), delimiter=',')

    for row in log_reader:
        # Data is logged in the format (timestamp, (can_id, data, len(data))).
        timestamp = row[0]
        can_id = int(row[1], 16)
        can_data = int(row[2], 16)
        data_len = int(row[3])

        msg = CanMessage(can_id, can_data.to_bytes(data_len, 'big'))

        print(timestamp)
        msg.parse()


def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(description='Parsing CAN log data')
    parser.add_argument('--file', required=True)

    args = parser.parse_args()

    parse_data(args.file)


if __name__ == '__main__':
    main()
