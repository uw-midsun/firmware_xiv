import time
from repl_setup import setup_default_channel
from gpio_set import gpio_set

setup_default_channel()

leds = [
    {'port': 'b', 'pin': 5, 'state': 0},
    {'port': 'b', 'pin': 4, 'state': 0},
    {'port': 'b', 'pin': 3, 'state': 0},
    {'port': 'a', 'pin': 13, 'state': 0},
]

while True:
    for led in leds:
        gpio_set(led['port'], led['pin'], led['state'])
        led['state'] = 1 if led['state'] == 0 else 0
        time.sleep(0.05)
