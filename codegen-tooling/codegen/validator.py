"""Validation things"""
from __future__ import absolute_import, division, print_function, unicode_literals

from constants import NUM_CAN_MESSAGES


def valid_can_id(can_id):
    """Determines whether a CAN id is valid

    Args:
        can_id: a CAN id value to test

    Returns:
        True if an can_id is a valid id in the range, or False
    """
    return 0 <= can_id < NUM_CAN_MESSAGES
