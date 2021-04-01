import unittest
import time

import sys
sys.path.append('projects/baby_driver/scripts')

from mpxe.integration_tests import int_test
import adc_read
import can_util
import gpio_get
import gpio_set

import repl_setup
import can_util

class TestBabyDriver(int_test.IntTest):
    def setUp(self):
        super().setUp()
        self.babydriver = self.manager.start('baby_driver')
        can_util.default_channel = "vcan0"
        repl_setup.call_gpio_it()
       

    def test_babydriver_adc_read(self):
        time.sleep(1)
        adc_read.adc_read('A', 6, True) # works when true, throws "ValueError: next_message expected babydriver ID (0,) but got 5" when False
        
    def test_can_util(self):
        time.sleep(1)
        can_util.send_message(2, [3,4,5], None, 63, 15)
        
            
    
if __name__ == '__main__':
    unittest.main()
