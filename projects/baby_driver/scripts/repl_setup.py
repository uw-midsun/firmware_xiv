"""
This module is run just before the Python REPL opens when you run `make babydriver`.
It imports everything that's visible to the REPL by default and sets up the default CAN channel.
"""

# pylint: disable=unused-import
from can_util import get_bus, send_message, next_message
from message_defs import BABYDRIVER_DEVICE_ID, BABYDRIVER_CAN_MESSAGE_ID, BabydriverMessageId


def setup_default_channel():
    """Set up the default CAN channel."""

    # We import these here so that they aren't imported for the REPL
    # pylint: disable=import-outside-toplevel
    import argparse
    import can_util

    parser = argparse.ArgumentParser(description="Setup the Babydriver REPL")
    parser.add_argument("--channel", default=None, help="Default CAN channel to use")

    args = parser.parse_args()

    if args.channel is not None:
        can_util.default_channel = args.channel


if __name__ == "__main__":
    setup_default_channel()
