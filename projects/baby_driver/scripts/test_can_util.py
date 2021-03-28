"""This Module Tests methods in can_util.py"""
import unittest

from can_util import can_pack


class TestCanPack(unittest.TestCase):
    """Test Can Util data_pack function"""

    def test_low_bound(self):
        """Tests Minimum values"""
        self.assertEqual([0], can_pack([(0, 1)]))
        self.assertEqual([0, 0, 0, 0, 0, 0, 0, 0], can_pack([(0, 8)]))
        self.assertEqual([1], can_pack([(1, 1)]))
        self.assertEqual([1, 0, 0, 0, 0, 0, 0, 0], can_pack([(1, 8)]))

    def test_high_bound(self):
        """Tests Maximum values"""
        self.assertEqual([255], can_pack([(255, 1)]))
        hex_val = int("ffffffffffffffff", 16)
        # 8-byte val is arbitrary, can_pack has no limit on output length
        self.assertEqual([255, 255, 255, 255, 255, 255, 255, 255], can_pack([(hex_val, 8)]))

    def test_multiple_inputs(self):
        """Tests with multiple value lists"""
        self.assertEqual([0, 0], can_pack([(0, 1), (0, 1)]))
        zero_list = [(0, 1), (0, 1), (0, 1), (0, 1), (0, 1), (0, 1), (0, 1), (0, 1)]
        self.assertEqual([0, 0, 0, 0, 0, 0, 0, 0], can_pack(zero_list))
        self.assertEqual([255, 255], can_pack([(255, 1), (255, 1)]))
        max_list = [(255, 1), (255, 1), (255, 1), (255, 1), (255, 1), (255, 1), (255, 1), (255, 1)]
        self.assertEqual([255, 255, 255, 255, 255, 255, 255, 255], can_pack(max_list))
        # test some arbitrary values - switch to little endian properly
        self.assertEqual([239, 205, 171, 0], can_pack([(int('ABCDEF', 16), 4)]))
        self.assertEqual([1, 2, 3], can_pack([(1, 1), (2, 1), (3, 1)]))
        self.assertEqual([27, 182, 100, 0], can_pack([(27, 1), (182, 1), (100, 2)]))
        arbitrary_list = [(365, 2), (99, 1), (12093847, 3)]
        self.assertEqual([109, 1, 99, 151, 137, 184], can_pack(arbitrary_list))
        outlist = [239, 205, 171, 0, 3, 2, 1]
        self.assertEqual(outlist, can_pack([(int('ABCDEF', 16), 4), (int('010203', 16), 3)]))

    def test_fail_conditions(self):
        """Tests fail conditions"""
        # test that only tuples of length 2 get through
        self.assertRaises(ValueError, can_pack, [(1, 1, 1)])
        self.assertRaises(ValueError, can_pack, [(1, 1), (1, 1, 1)])
        # test possible illegal inputs
        self.assertRaises(ValueError, can_pack, [(1, 0)])
        self.assertRaises(ValueError, can_pack, [(1, 1), (1, 0)])
        self.assertRaises(ValueError, can_pack, [(-1, 1)])
        self.assertRaises(ValueError, can_pack, [(1, -1)])
        # test that val not greater than max allowed by number of bytes
        self.assertRaises(ValueError, can_pack, [(256, 1)])
        self.assertRaises(ValueError, can_pack, [(65536, 2)])


if __name__ == '__main__':
    unittest.main()
