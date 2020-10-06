import pm
import time
import mocks.controller_board_blinking_leds as cb_leds

# This will eventually not be used, since we'll
# use this as a library for test scripts to call
if __name__ == "__main__":
    # pm.init()
    manager = pm.ProjectManager()
    manager.statuses['controller_board_blinking_leds'] = True
    # manager.build('controller_board_blinking_leds')
    leds = manager.start('controller_board_blinking_leds')
    time.sleep(1)
    manager.stop(leds)
    manager.end()

