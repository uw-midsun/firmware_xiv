import time
import unittest
from mpxe.integration_tests import int_test
from mpxe.sims import sim
import sys
sys.path.append('projects/baby_driver/scripts')
import adc_read
import can_util
import gpio_get
import gpio_interrupts
import spi_exchange
import i2c_write
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
        self.babydriver.sim.get_gpio(proj=self.babydriver, port='A', pin=5)
        time.sleep(1)
        gpio_set.gpio_set('A', 5, True)
        time.sleep(1)
        self.babydriver.sim.get_gpio(proj=self.babydriver, port='A', pin=5)
    def test_gpio_interrupts(self):
        time.sleep(1)
        gpio_interrupts.register_gpio_interrupt(port='A', pin=3)
        time.sleep(1)
        gpio_interrupts.unregister_gpio_interrupt(port='A', pin=3)
    def test_i2c_write(self):
        time.sleep(1)
        i2c_write.i2c_write(port=1, address=200, tx_bytes=[1,2,3,4,5,6], reg=None)
    def test_spi_exchange(self):
        time.sleep(1)
        spi_exchange.spi_exchange(tx_bytes=[1,2,3,4,5,6], rx_len=6, spi_port=1, spi_mode=0, baudrate=6000000, cs=None)
if __name__ == '__main__':
    unittest.main()