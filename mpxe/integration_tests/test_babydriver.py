import sys
sys.path.append('projects/baby_driver/scripts')

import time
import unittest
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
        time.sleep(1)
        adc_read.adc_read(port='A', pin=6, raw=True)

    def test_gpio_get(self):
        time.sleep(1)
        gpio_get.gpio_get('A', 6)

    def test_gpio_set(self):
        time.sleep(1)
        gpio_set.gpio_set('A', 5, True)
        
        time.sleep(0.5)
        self.babydriver.sim.get_gpio(proj=self.babydriver, port='A', pin=5)
        ''' Throws an error of KeyError: (1,0) '''

    def test_gpio_interrupts(self):
        time.sleep(1)
        gpio_interrupts.register_gpio_interrupt(port='A', pin=3)
        
        time.sleep(1)
        gpio_interrupts.unregister_gpio_interrupt(port='A', pin=3)
        
if __name__ == '__main__':
    unittest.main()
