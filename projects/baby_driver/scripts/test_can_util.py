import unittest

from can_util import can_pack

class TestCanPack(unittest.TestCase):
    """ Test Can Util data_pack function """
    def test_low_bound(self):
        self.assertEqual(b'\x00', can_pack(data_list=[(0,1)]))
        self.assertEqual(b'\x00\x00\x00\x00\x00\x00\x00\x00', can_pack(data_list=[(0,8)]))
        self.assertEqual(b'\x01', can_pack(data_list=[(1,1)]))
        self.assertEqual(b'\x00\x00\x00\x00\x00\x00\x00\x01', can_pack(data_list=[(1,8)]))
    
    def test_high_bound(self):
        self.assertEqual(b'\xFF', can_pack(data_list=[(255,1)]))
        hex_val = int("ffffffffffffffff", 16)
        #8-byte val is arbitrary, can_pack has no limit on output length
        self.assertEqual(b'\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF', can_pack(data_list=[(hex_val,8)]))

    def test_multiple_inputs(self):
        self.assertEqual(b'\x00\x00', can_pack(data_list=[(0,1), (0,1)]))
        self.assertEqual(b'\x00\x00\x00\x00\x00\x00\x00\x00',
                can_pack(data_list=[(0,1),(0,1),(0,1),(0,1),(0,1),(0,1),(0,1),(0,1),]))
        self.assertEqual(b'\xFF\xFF', can_pack(data_list=[(255,1),(255,1)]))
        self.assertEqual(b'\xFF\xFF\xFF\xFF\xFF\xFF\xFF\xFF', 
                can_pack(data_list=[(255,1),(255,1),(255,1),(255,1),(255,1),(255,1),(255,1),(255,1)]))
        #test some arbitrary values
        self.assertEqual(b'\x1b\xb6\x27\x10', can_pack(data_list=[(27,1),(182,1),(10000, 2)]))
        self.assertEqual(b'\x01\x6d\x63\xb8\x89\x97', can_pack(data_list=[(365,2),(99,1),(12093847, 3)]))

    def test_fail_conditions(self):
        #test that only tuples of length 2 get through
        self.assertRaises(ValueError, can_pack, [(1,1,1)])
        self.assertRaises(ValueError, can_pack, [(1,1),(1,1,1)])
        #test possible illegal inputs
        self.assertRaises(ValueError, can_pack, [(1,0)])
        self.assertRaises(ValueError, can_pack, [(1,1), (1,0)])
        self.assertRaises(ValueError, can_pack, [(-1,1)])
        self.assertRaises(ValueError, can_pack, [(1,-1)])
        #test that val not greater than max allowed by number of bytes
        self.assertRaises(ValueError, can_pack, [(256,1)])
        self.assertRaises(ValueError, can_pack, [(65536, 2)])


if __name__ == '__main__':
    unittest.main()