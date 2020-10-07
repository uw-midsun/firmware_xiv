import can
import cantools
import time

bus = can.interface.Bus('vcan0', bustype='socketcan')
db = cantools.database.load_file('../system_can.dbc')

pedal_msg = db.get_message_by_name('PEDAL_OUTPUT')

def send_val(val):
    data = pedal_msg.encode({'throttle_output': int(val), 'brake_output': 0})
    msg = can.Message(arbitration_id=pedal_msg.frame_id, extended_id=False, data=data)
    # print('sending {}'.format(val))
    bus.send(msg)

# precharge
print('sending precharge')
bus.send(can.Message(arbitration_id=db.get_message_by_name('BEGIN_PRECHARGE').frame_id, extended_id=False))
time.sleep(1)

# drive state
print('sending drive message')
bus.send(can.Message(arbitration_id=db.get_message_by_name('DRIVE_OUTPUT').frame_id, extended_id=False, data=[1, 0]))
time.sleep(1)

tick = 0.0
max_tick = 10
while True:
    send_val(float(tick / max_tick) * 100)
    tick += 1
    if tick > max_tick:
        tick = 0
    time.sleep(3)