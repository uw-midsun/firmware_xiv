# -*- coding: utf-8 -*-
"""A CAN message parsing utility module"""
import binascii
import struct

DATA_POWER_STATE = ['idle', 'charge', 'drive']
LIGHTS_ID_NAME = [
    'High beams', 'Low beams', 'DRL', 'Brakes',
    'Left Turn', 'Right Turn', 'Hazards', 'BPS Strobe'
]


def data_relay(state):
    """Relay state data format"""
    return 'close' if state else 'open'


def data_power_state(state):
    """Power state data format"""
    return DATA_POWER_STATE[state]


def data_lights_state(light_id, state):
    """Lights state data format"""
    id_name = LIGHTS_ID_NAME[light_id]
    state_name = 'on' if state else 'off'
    return '{}: {}'.format(id_name, state_name)


def data_battery_vt(module, voltage, temperature):
    """Battery V/T data format"""
    return 'C{}: {:.1f}mV aux {:.1f}mV'.format(module, voltage / 10, temperature / 10)


def data_battery_voltage_current(voltage, current):
    """Battery total voltage/current data format"""
    return '{:.4f}V {:.4f}A'.format(voltage / 10000, current / 1000000)


def data_dump(*args):
    """Generic data dump format"""
    return ' '.join(['{}'.format(arg) for arg in args])


SOURCE_LOOKUP = [
    'RESERVED',
    'PLUTUS',
    'PLUTUS_SLAVE',
    'CHAOS',
    'TELEMETRY',
    'LIGHTS_FRONT',
    'LIGHTS_REAR',
    'MOTOR_CONTROLLER',
    'DRIVER_CONTROLS_PEDAL',
    'DRIVER_CONTROLS_CENTER_CONSOLE',
    'SOLAR_MASTER_FRONT',
    'SOLAR_MASTER_REAR',
    'SENSOR_BOARD',
    'CHARGER',
    'DRIVER_CONTROLS_STEERING',
    'NUM_SYSTEM_CAN_DEVICES'
]

# Name, struct format, data format 0, ...
MESSAGE_LOOKUP = {
    0: ('BPS Heartbeat', '<B', data_dump),
    1: ('Chaos Fault', '<B', data_dump),
    2: ('Battery relay (Main)', '<B', data_relay),
    3: ('Battery relay (Slave)', '<B', data_relay),
    4: ('Motor relay', '<B', data_relay),
    5: ('Solar relay (Rear)', '<B', data_relay),
    6: ('Solar relay (Front)', '<B', data_relay),
    7: ('Power state', '<B', data_power_state),
    8: ('Chaos Heartbeat', '', data_dump),
    18: ('Drive Output', '<hhhh', data_dump),
    19: ('Cruise Target', '<B', data_dump),
    23: ('Lights Sync', '', data_dump),
    24: ('Lights State', '<BB', data_lights_state),
    26: ('Charger state', '<B', data_dump),
    27: ('Charger relay', '<B', data_relay),
    32: ('Battery V/T', '<HHH', data_battery_vt),
    33: ('Battery Voltage/Current', '<ii', data_battery_voltage_current),
    35: ('Motor Bus Measurement', '<hhhh', data_dump),
    36: ('Motor Velocity', '<hh', data_dump),
    43: ('Aux & DC/DC V/C', '<HHHH', data_dump),
}


class CanMessage:
    """A representation of a CAN Message

    This is simply a CAN ID and the CAN data

    Args:
        can_id: Raw standard CAN ID.
        can_data: Message data. Up to 8 bytes.
    """

    def __init__(self, can_id, can_data):
        self._can_id = can_id
        self._can_data = can_data

    @property
    def source_id(self):
        """The source ID of a CAN message is the original sender"""
        return self._can_id & 0xf

    @property
    def msg_type(self):
        """Either CAN_MSG_TYPE_DATA (0) or CAN_MSG_TYPE_ACK (1)"""
        return (self._can_id >> 4) & 0x1

    @property
    def msg_id(self):
        """The message ID as specified in codegen"""
        return (self._can_id >> 5) & 0x3f

    def parse(self):
        """Parse the given CAN ID and data"""
        # System CAN ID format:
        # [0:3] Source ID
        # [4] Message Type (ACK/DATA)
        # [5:10] Message ID
        msg_type_name = 'ACK' if self.msg_type == 1 else 'DATA'

        if self.msg_id in MESSAGE_LOOKUP:
            name, fmt, data_fn = MESSAGE_LOOKUP[self.msg_id]
            if fmt:
                try:
                    unpacked_data = struct.unpack(fmt, self._can_data)
                except struct.error:
                    print('Invalid {}'.format(self.msg_id))
                    return
            else:
                unpacked_data = []

            if msg_type_name == 'ACK':
                source = self.source_id
                if self.source_id in MESSAGE_LOOKUP:
                    source = SOURCE_LOOKUP[self.source_id]
                print('{} ACK from {}'.format(name, source))
            else:
                print('{}: {}'.format(name, data_fn(*unpacked_data)))
        else:
            # Unrecognized message
            print('{} from {} ({}): 0x{}'.format(self.msg_id, self.source_id, msg_type_name,
                                                 binascii.hexlify(self._can_data).decode('ascii')))
