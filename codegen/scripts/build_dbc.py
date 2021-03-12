"""
This script generates a DBC file from our custom protobufs in order to make the
transition to DBCs more seamless. This is a temporary measure so we can
continue to use codegen-tooling, while switching over to using DBC decoders so
we do not have to maintain the dump scripts.

The idea is that we will eventually switch over to DBC files, or a higher level
DSL that compiles down into DBC files (ie. if our CAN protocol changes).
"""
from __future__ import absolute_import, division, print_function, unicode_literals

import cantools

# For the proto and asciipb parsing
import data

# Length of fields (in bits)
FIELDS_LEN = {
    'u8': 8,
    'u16': 16,
    'u32': 32,
    'u64': 64
}

# pylint: disable=W0511
SIGNED_MESSAGES = []
ACKABLE_MESSAGES = {}


def build_arbitration_id(msg_type, source_id, msg_id):
    """
    typedef union CanId {
      uint16_t raw;
      struct {
        uint16_t source_id : 4;
        uint16_t type : 1;
        uint16_t msg_id : 6;
      };
    } CanId;
    """
    return ((source_id & ((0x1 << 4) - 1)) << (0)) | \
        ((msg_type & ((0x1 << 1) - 1)) << (4)) | \
        ((msg_id & ((0x1 << 6) - 1)) << (4 + 1))


def main():
    """The main entry-point of the program"""
    # pylint: disable=R0914
    database = cantools.database.can.Database(version='')

    # Iterate through all the CAN device IDs and add them to the DBC.
    can_devices = data.parse_can_device_enum()
    for device_id, device_name in can_devices.items():
        if device_name not in ['RESERVED']:
            node = cantools.database.can.Node(
                name=device_name,
                comment='Device ID: {}'.format(hex(device_id))
            )
            database.nodes.append(node)

    # Iterate through all the CAN messages IDs and:
    #
    #   1. Convert the Message ID to the CAN Arbitration ID.
    #   2. Add the Message to the DBC.
    #   3. For each of the signals, add it to the Message.
    #   4. If the Message is critical, add an ACK message to the DBC.
    can_messages = data.parse_can_frames('can_messages.asciipb')
    device_enum = data.parse_can_device_enum()

    def get_key_by_val(dictionary, val):
        """Helper function to get key for dictionary value"""
        for key, value in dictionary.items():
            if val == value:
                return key
        return None

    for msg_id, can_frame in can_messages.items():
        source = get_key_by_val(device_enum, can_frame.source)
        # Checks for critical messages to make sure an ACK is added later
        if can_frame.is_critical is not None and can_frame.is_critical:
            ACKABLE_MESSAGES[msg_id] = ((can_frame.target).replace(" ", "")).split(",")
        # Checks for signed messages
        if can_frame.is_signed is not None and can_frame.is_signed:
            SIGNED_MESSAGES.append(str(can_frame.msg_name))

        # All these message types must be Data messages. ACK messages are
        # currently handled implicitly by the protocol layer, and will be
        # generated based on whether or not it is an ACKable message.
        frame_id = build_arbitration_id(
            msg_type=0,
            source_id=source,
            msg_id=msg_id
        )

        total_length = 0
        signals = []
        for index, field in enumerate(can_frame.fields):
            length = FIELDS_LEN[can_frame.ftype]

            if can_frame.msg_name in SIGNED_MESSAGES \
                    and not field == 'voltage':
                signal = cantools.database.can.Signal(
                    name=field,
                    start=index * length,
                    length=length,
                    is_signed=True
                )
            else:
                # battery voltage is unsigned
                signal = cantools.database.can.Signal(
                    name=field,
                    start=index * length,
                    length=length,
                    is_signed=False
                )
            signals.append(signal)
            total_length += length

        # Note: It is safe to divide by 8 since every single message under
        # the old protocol (aka. what I call CANdlelight 1.0) is
        # byte-aligned. To be precise, it uses byte-alignment padding to
        # fit 8, 4, 2, 1 bytes.
        message = cantools.database.can.Message(
            frame_id=frame_id,
            name=can_frame.msg_name,
            length=total_length // 8,
            signals=signals,
            # The sender is the Message Source
            senders=[can_frame.source]
        )
        database.messages.append(message)

        def get_ack(sender, msg_name, msg_id):
            """
            msg_id: the id of the message we are ACKing
            """
            sender_id = get_key_by_val(device_enum, sender)
            if sender_id is None:
                print("Couldn't find {}".format(sender))

            frame_id = build_arbitration_id(
                msg_type=1,
                source_id=sender_id,
                msg_id=msg_id
            )

            # All ACK responders send a message containing a ACK_STATUS in
            # CANdlelight 1.0
            signals = [
                cantools.database.can.Signal(
                    name='{}_FROM_{}_ACK_STATUS'.format(msg_name, sender),
                    start=0,
                    length=8
                )
            ]
            # This ACK message is always length 1, since it just fits the
            # ACK_STATUS
            message = cantools.database.can.Message(
                frame_id=frame_id,
                name='{}_ACK_FROM_{}'.format(msg_name, sender),
                length=1,
                signals=signals,
                # The sender is the Message Source
                senders=[sender]
            )
            return message

        # If this requires an ACK, then we go through all of the receivers.
        if msg_id in ACKABLE_MESSAGES:
            for acker in ACKABLE_MESSAGES[msg_id]:
                if acker != "":
                    message = get_ack(str(acker), can_frame.msg_name, msg_id)
                database.messages.append(message)

    # Save as a DBC file
    with open('system_can.dbc', 'w') as file_handle:
        file_handle.write(database.as_dbc_string())


if __name__ == '__main__':
    main()
