from mu.harness.board_sim import BoardSim
from mu.integration_tests import int_test
import repl_setup
import gpio_set
import i2c_write
import spi_exchange
import gpio_interrupts
import gpio_get
import can_util
import adc_read
import time
import unittest
import sys
sys.path.append('projects/baby_driver/scripts')
# pylint: disable=wrong-import-position


class TestBabyDriver(int_test.IntTest):
    def setUp(self):
        super().setUp()
        self.babydriver = self.manager.start(BoardSim, proj_name='baby_driver')
        can_util.default_channel = "vcan0"
        repl_setup.initialize_modules()

    def test_adc_read(self):
        time.sleep(1)
        adc_read.adc_read(port='A', pin=6, raw=False)

    def test_gpio_get(self):
        time.sleep(1)
        self.babydriver.set_gpio(port='A', pin=5, state=False)
        time.sleep(1)
        assert gpio_get.gpio_get('A', 5) is False

    def test_gpio_set(self):
        time.sleep(1)
        self.babydriver.set_gpio(port='A', pin=5, state=False)
        time.sleep(1)
        gpio_set.gpio_set('A', 5, True)
        time.sleep(1)
        assert self.babydriver.get_gpio(port='A', pin=5) is True

    def test_gpio_interrupts(self):
        # Defining a Callback function
        isCalled = False

        def interrupt_callback(info):
            nonlocal isCalled
            isCalled = True
        # Registering the interrupt
        time.sleep(1)
        gpio_interrupts.register_gpio_interrupt(port='A',
                                                pin=5,
                                                callback=interrupt_callback)
        # Simulating
        time.sleep(1)
        self.babydriver.set_gpio(port='A', pin=5, state=True)
        time.sleep(1)
        self.babydriver.set_gpio(port='A', pin=5, state=False)
        # Asserting
        assert isCalled is True
        # Unregistering the interrupt
        time.sleep(1)
        gpio_interrupts.unregister_gpio_interrupt(port='A', pin=5)

    def test_i2c_write(self):
        time.sleep(1)
        i2c_write.i2c_write(port=1, address=200, tx_bytes=[1, 2, 3, 4, 5, 6], reg=None)

    def test_spi_exchange(self):
        time.sleep(1)
        spi_exchange.spi_exchange(
            tx_bytes=[
                1,
                2,
                3,
                4,
                5,
                6],
            rx_len=6,
            spi_port=1,
            spi_mode=0,
            baudrate=6000000,
            cs=None)


if __name__ == '__main__':
    unittest.main()
