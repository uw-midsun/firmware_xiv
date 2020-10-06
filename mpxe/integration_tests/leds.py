import sys
from os.path import dirname
sys.path.append(dirname(sys.path[0]))

import time

from harness import pm
from harness import project

manager = pm.ProjectManager()
manager.statuses['controller_board_blinking_leds'] = True
# manager.build('controller_board_blinking_leds')
leds = manager.start('controller_board_blinking_leds')
time.sleep(1)
manager.stop(leds)
manager.end()

print('PASS leds')
sys.exit(0)
