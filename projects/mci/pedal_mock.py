import can
import cantools
import time
import argparse

bus = can.Bus('slcan0', bustype='socketcan')
db = cantools.database.load_file('../../system-can.dbc')

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

while(1):
    send_pedal(args.brake, args.throttle)
    time.sleep(0.1)
