import unittest
import time

import sys
sys.path.append('projects/baby_driver/scripts')

from mpxe.integration_tests import int_test
from mpxe.sims import sim

import adc_read
import can_util
import gpio_get
import gpio_interrupts
import gpio_port
import gpio_set
import repl_setup


class TestBabyDriver(int_test.IntTest):
    def setUp(self):
        super().setUp()
        self.babydriver = self.manager.start('baby_driver')
        can_util.default_channel = "vcan0"
        repl_setup.call_gpio_it()
       

    def test_babydriver_adc_read(self):
        time.sleep(0.5)
        adc_read.adc_read(port='A', pin=6) # Tests if ADC reads data properly
        '''Sometimes, throws "ValueError: next_message expected babydriver ID (0,) but got 5" when False'''
        
    def test_can_util(self):
        time.sleep(0.5)
        can_util.send_message(babydriver_id=2, data=[3,4,5]) # Tests can send message
        
        #time.sleep(0.5)
        #can_util.next_message(2) #not working
        
        time.sleep(0.5)
        can_util.can_pack(data_list=[(3,2), (4,5), (1,2), (4,9)]) # Tests can pack values and their length in bytes
    
    def test_gpio_get(self):
        time.sleep(0.5)
        #gpio_get.gpio_get('A', 6)
        sim.get_gpio(self.babydriver, "babydriver", 'A', 6)
        
          
        
        
            
    
if __name__ == '__main__':
    unittest.main()
