import sys
from os.path import dirname
sys.path.append(dirname(sys.path[0]))

import time

from harness import pm
from harness import project
from harness import protogen

manager = pm.ProjectManager()
manager.statuses['pedal_board'] = True
# manager.build('pedal_board')
pedal = manager.start('pedal_board')

time.sleep(0.5)
pedal.handler.update_ads_reading(pedal, 50, 1)

time.sleep(1)
manager.stop(pedal)
manager.end()

print('PASS pedal')
sys.exit(0)
