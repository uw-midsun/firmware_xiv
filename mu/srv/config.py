import argparse
from collections import namedtuple

Config = namedtuple('Config', ['background', 'canbus'])

def get_config():
    parser = argparse.ArgumentParser(prog='musrv', description='Run MU as a server')
    parser.add_argument('-bg', dest='background', action='store_true',
                        help='run musrv in the background')
    parser.add_argument('-cb', dest='canbus', default='vcan0', help='set canbus name')
    return parser.parse_args()
