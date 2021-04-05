import time
import unittest
from mpxe.integration_tests import int_test
from mpxe.sims import sim
from mpxe.sims.sim import Sim
import adc_read
import can_util
import gpio_get
import gpio_interrupts
import gpio_port
import gpio_set
import repl_setup
import sys
sys.path.append('projects/baby_driver/scripts')


class TestBabyDriver(int_test.IntTest):
    def setUp(self):
        super().setUp()
        self.babydriver = self.manager.start('baby_driver')
        can_util.default_channel = "vcan0"
        repl_setup.call_gpio_it()

    def test_babydriver_adc_read(self):
        time.sleep(0.5)
        adc_read.adc_read(port='A', pin=6)

    def test_gpio_get(self):
        time.sleep(0.5)
        gpio_get.gpio_get('A', 6)

    def test_gpio_set(self):
        time.sleep(0.5)
        gpio_set.gpio_set('A', 5, True)

    def test_gpio_interrupts(self):
        time.sleep(0.5)
        gpio_interrupts.register_gpio_interrupt(port='A', pin=3)


if __name__ == '__main__':
    unittest.main()
