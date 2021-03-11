"""Module for testing validator methods."""
from __future__ import absolute_import, division, print_function, unicode_literals

import unittest

import validator
from constants import NUM_CAN_MESSAGES


class TestValidatorMethods(unittest.TestCase):
    """Tests the validator module methods."""

    def test_valid_can_id_in_range(self):
        """Tests if a valid can message is in range."""
        for can_msg_id in range(0, NUM_CAN_MESSAGES):
            self.assertTrue(validator.valid_can_id(can_msg_id))

    def test_valid_can_id_out_of_range(self):
        """Tests if a valid can message is out of range."""
        self.assertFalse(validator.valid_can_id(NUM_CAN_MESSAGES))
