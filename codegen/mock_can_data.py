# This script automatically generates and sends CAN messages based off of the DBC file
# Ensure that you have can and cantools installed, which can be accomplished by running
# pip install -r requirements.txt
# Alternatively, you could also just run
# pip install can && pip install cantools

# If you are running this script with virtual CAN ensure that it is set up first
# You can run the below commands
# sudo modprobe vcan
# sudo ip link add dev vcan0 type vcan
# sudo ip link set up vcan0

import cantools
import can
import time
import random

# A negative value for num_messages will cause the script to send CAN messages forever
num_messages = -1
sleep_time_s = 1
can_messages = []

try:
    db = cantools.database.load_file('system_can.dbc')
except:
    print("Must generate DBC file first")
    print("Run make gen && make gen-dbc")

# This can be edited depending on the CAN interface
can_bus = can.interface.Bus('vcan0', bustype='socketcan')

def main():
    get_messages()
    iterate_message_and_signal()

def get_messages():
    for msg in db.messages:
        can_messages.append(msg)

def iterate_message_and_signal():
    num_messages_sent = 0
    while num_messages < 0 or num_messages > num_messages_sent:
        try:
            send_message()
            num_messages_sent +=1
            time.sleep(sleep_time_s)
        except KeyboardInterrupt:
            break
    print("\n" + str(num_messages_sent) + " CAN messages have been sent")

def send_message():
    msg = random.choice(can_messages)
    data = {}
    for signal in msg.signals:
        data[signal.name]=random.randint(0, pow(2,signal.length)-1)
    new_data = msg.encode(data)
    message = can.Message(arbitration_id=msg.frame_id, data=new_data)
    can_bus.send(message)

if __name__ == "__main__":
    main()

