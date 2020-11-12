import can
import cantools
import time

bus = can.interface.Bus('can0', bustype='socketcan')
db = cantools.database.load_file('../system_can.dbc')

def turn_on():
    relay_on_msg = db.get_message_by_name('SET_RELAY_STATES')
    data = relay_on_msg.encode({'relay_mask': 1, 'relay_state': 1})
    msg = can.Message(arbitration_id=relay_on_msg.frame_id, extended_id=False, data=data)
    bus.send(msg)

def turn_off():
    relay_off_msg = db.get_message_by_name('SET_RELAY_STATES')
    data = relay_off_msg.encode({'relay_mask': 1, 'relay_state': 0})
    msg = can.Message(arbitration_id=relay_off_msg.frame_id, extended_id=False, data=data)
    bus.send(msg)

time.sleep(1)

print('sending turn on')
turn_on()
time.sleep(2)

print('sending turn off')
turn_off()
