"""
This script is run just before the Python REPL opens when you run `make babydriver`.
It imports everything that's visible to the REPL by default and sets up the default CAN channel.
"""


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

    print("Welcome to the Babydriver REPL!")
    print("See https://uwmidsun.atlassian.net/l/c/XDCix3iH for details and usage.")


if __name__ == "__main__":
    setup_default_channel()
