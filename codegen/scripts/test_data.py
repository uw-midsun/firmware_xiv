"""Module for testing the proto data reader."""
from __future__ import absolute_import, division, print_function, unicode_literals

import unittest
import mock

from parameterized import parameterized

import data


# pylint: disable=invalid-name
class TestDataMethods(unittest.TestCase):
    """Test the proto parsing and data reading."""

    @parameterized.expand([("""
            msg {
                id:300
                msg_name: "can id over 128"
                can_data {
                    frame {
                        type: UINT64
                    }
                }
            }
        """), ("""
            msg {
                id: 1
                msg_name: "can message 1"
                can_data {
                    empty {
                    }
                }
            }

            msg {
                id: 1
                msg_name: "silly duplicate message id"
                can_data {
                    empty {
                    }
                }
            }
        """), ("""
            msg {
                id: 1
                msg_name: "some name"
                can_data {
                    empty {
                    }
                }
            }

            msg {
                id: 3
                msg_name: "some name"
                can_data {
                    empty {
                    }
                }
            }
        """)])
    @mock.patch("data.open")
    def test_parse_can_message_enum_exceptions(self, ascii_protobuf,
                                               mock_open):
        """Tests for an invalid CAN ID handler."""
        mock_open.side_effect = [
            mock.mock_open(read_data=ascii_protobuf).return_value
        ]
        with self.assertRaises(Exception):
            data.parse_can_message_enum(None)

    @parameterized.expand([("""
            msg {
                id:300
                msg_name: "can id over 128"
                can_data {
                    frame {
                        type: UINT64
                    }
                }
            }
        """), ("""
            msg {
                id: 1
                msg_name: "can message 1"
                can_data {
                    empty {
                    }
                }
            }

            msg {
                id: 1
                msg_name: "silly duplicate message id"
                can_data {
                    empty {
                    }
                }
            }
        """), ("""
            msg {
                id: 1
                msg_name: "some name"
                can_data {
                    u8 {
                        field_name_1: "hello world"
                        field_name_2: "hello world"
                    }
                }
            }
        """)])
    @mock.patch("data.open")
    def test_parse_can_frames_exceptions(self, ascii_protobuf, mock_open):
        """Tests for an invalid CAN ID handler."""
        mock_open.side_effect = [
            mock.mock_open(read_data=ascii_protobuf).return_value
        ]
        with self.assertRaises(Exception):
            data.parse_can_frames(None)


if __name__ == '__main__':
    unittest.main()
