import can
import cantools
import threading
from collections import deque

class Msg:
    def __init__(self, name, arb_id, data):
        self.name = name
        self.arb_id = arb_id
        self.data = data

class Canio:
    def __init__(self, bus_name, max_msgs=10, callbacks):
        self.messages = deque(maxlen=max_msgs)
        self.db = cantools.database.load_file('../system_can.dbc')
        self.bus = can.interface.Bus(bus_name, receive_own_messages=True, bustype='socketcan')
        self.killed = False
        self.callbacks = callbacks
        self.listen_thread = threading.Thread(target=self.listener)
        self.listen_thread.start()

    def get_latest_by_name(self, name):
        for msg in self.messages:
            if msg.name == name:
                return msg

    def send(self, name, data):
        msg_type = self.db.get_message_by_name(name)
        encoded_data = msg_type.encode(data)
        msg = can.Message(arbitration_id=msg_type.frame_id, 
            is_extended_id=False, data=encoded_data)
        self.bus.send(msg)

    def stop(self):
        self.killed = True
        self.listen_thread.join()
        self.bus.shutdown()

    def listener_callback(self, msg):
        print('[CAN] {}#{}: {}'.format(msg.name, msg.arb_id, msg.data))

    def listener(self):
        while not self.killed:
            raw_msg = self.bus.recv(timeout=0.5)
            if raw_msg == None:
                continue
            msg = Msg('UNKOWN', raw_msg.arbitration_id, raw_msg.data)
            try:
                msg.name = self.db.get_message_by_frame_id(raw_msg.arbitration_id)
                msg.data = self.db.decode_message(raw_msg.arbitration_id, raw_msg.data)
                if len(self.messages)  == self.messages.maxlen:
                    self.messages.pop()
                self.messages.appendleft()
            except KeyError as e:
                pass
            self.messages.appendleft(msg)
            for cb in self.callbacks:
                cb(msg)
