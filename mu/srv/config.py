import argparse
from collections import namedtuple

Config = namedtuple('Config', ['background', 'canbus', 'projlogs'])

def get_config():
    parser = argparse.ArgumentParser(prog='musrv', description='Run MU as a server')
    parser.add_argument('-bg', dest='background', action='store_true',
                        help='run musrv in the background')
    parser.add_argument('-cb', dest='canbus', default='vcan0', help='set canbus name')
    parser.add_argument('-pl', dest='projlogs', action='store_true',
                        help='output project logs')
    return parser.parse_args()
