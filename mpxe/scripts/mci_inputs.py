# quick script for mocking inputs to MCI
import time

from mpxe.harness.pm import ProjectManager
from mpxe.harness.project import Project
from mpxe.sims.pedal_board import PedalBoard

manager = ProjectManager()
pedal = manager.start('pedal_board', PedalBoard())
THROTTLE_CHANNEL = 1
time.sleep(1)

manager.can.send('BEGIN_PRECHARGE', {})
time.sleep(0.5)

manager.can.send('DRIVE_OUTPUT', {'drive_output': 1})
time.sleep(0.5)

tick = 0
max_tick = 10
try:
    while True:
        send_val = int(float(tick / max_tick) * 100)
        pedal.sim.update_ads_reading(pedal, send_val, THROTTLE_CHANNEL)
        tick += 1
        if tick > max_tick:
            tick = 0
        time.sleep(1)
except KeyboardInterrupt as k:
    manager.end()
    raise k
