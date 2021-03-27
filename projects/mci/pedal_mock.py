import can
import cantools
import time
import argparse
import os

bus = can.Bus('slcan0', bustype='socketcan')
# bus = can.Bus('can0', bustype='pcan') # missing a file or smth idk
db = cantools.database.load_file('../../system-can.dbc')

# sudo ip link set can0 up type can bitrate 500000

def send_pedal(brake, pedal):
    print("sending brake {} pedal {}".format(brake, pedal))
    pedal_on_msg = db.get_message_by_name("PEDAL_OUTPUT")
    data = pedal_on_msg.encode({'brake_output': int(brake), 'throttle_output': int(pedal)})
    msg = can.Message(arbitration_id=pedal_on_msg.frame_id, extended_id=False, data=data)
    bus.send(msg)

parser = argparse.ArgumentParser()
parser.add_argument("--brake", "-b")
parser.add_argument("--throttle", "-t")
args = parser.parse_args()

# while(1):
#     send_pedal(args.brake, args.throttle)
#     time.sleep(0.1)



# slowly speed up
for i in range(1, 20):
    for j in range(1, 10):
        send_pedal(0, i)
        time.sleep(0.1)
    
while(1):
    send_pedal(0, 20)
    time.sleep(0.1)

# send_pedal(10, 0)