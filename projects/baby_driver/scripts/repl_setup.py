"""
This module is run just before the Python REPL opens when you run `make babydriver`.
It imports everything that's visible to the REPL by default and sets up the default CAN channel.
"""

from can_message import get_bus, send_message, next_message, TimeoutError
from message_defs import BABYDRIVER_DEVICE_ID, BABYDRIVER_CAN_MESSAGE_ID, BabydriverMessageId


def main():
    # We import these in main so that they aren't imported for the REPL
    import argparse
    import can_message

    parser = argparse.ArgumentParser(description="Setup the Babydriver REPL")
    parser.add_argument("--channel", default=None, help="Default CAN channel to use")

    args = parser.parse_args()

    if args.channel is not None:
        can_message.default_channel = args.channel


if __name__ == "__main__":
    main()
