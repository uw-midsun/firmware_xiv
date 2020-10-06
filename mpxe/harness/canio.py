import can
import cantools
import threading
from collections import deque

class Msg:
    def __init__(self, metadata, data):
        self.metadata = metadata
        self.data = data

class Canio:
    def __init__(self, max_msgs=10):
        self.messages = deque(maxlen=max_msgs)
        self.db = cantools.database.load_file('../system_can.dbc')
        self.bus = can.interface.Bus('vcan0', receive_own_messages=True, bustype='socketcan')
        self.killed = False
        self.listen_thread = threading.Thread(target=self.listener)
        self.listen_thread.start()
    def get_msg_by_name(self, name):
        for msg in self.messages:
            if msg.metadata.name == name:
                return msg.data
    def send(self, name, data):
        msg_type = self.db.get_message_by_name(name)
        encoded_data = msg_type.encode(data)
        msg = can.Message(arbitration_id=msg_type.frame_id, data=encoded_data)
        self.bus.send(msg)
    def stop(self):
        self.killed = True
        self.listen_thread.join()
    def listener(self):
        while not self.killed:
            raw_msg = self.bus.recv(timeout=0.5)
            if raw_msg == None:
                continue
            msg_data = self.db.decode_message(raw_msg.arbitration_id, raw_msg.data)
            metadata = self.db.get_message_by_frame_id(raw_msg.arbitration_id)
            print('[CAN] {}: {}'.format(metadata.name, msg_data))
            if len(self.messages)  == self.messages.maxlen:
                self.messages.pop()
            self.messages.appendleft(Msg(metadata, msg_data))
