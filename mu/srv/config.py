import argparse
from collections import namedtuple

def get_config():
    parser = argparse.ArgumentParser()
    parser.add_argument('-bg', dest='background', action='store_false')
    parser.add_argument('-cb', dest='canbus', default='vcan0')
    return parser.parse_args()
