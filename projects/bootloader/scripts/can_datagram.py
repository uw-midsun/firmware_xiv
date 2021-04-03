"""This client script handles datagram protocol communication between devices on the CAN."""

import can


class Datagram:
    """This class implements the Datagram modules."""

    def __init__(self):
        print("Init")
        self.bus = can.interface.Bus()


can_datagram = Datagram()
