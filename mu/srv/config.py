import argparse
from collections import namedtuple

Config = namedtuple('Config', ['background', 'canbus'])

def get_config():
    parser = argparse.ArgumentParser()
    parser.add_argument('-bg', dest='background', action='store_true')
    parser.add_argument('-cb', dest='canbus', default='vcan0')
    return parser.parse_args()
