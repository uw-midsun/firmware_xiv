import threading
from collections import deque, namedtuple
import os

import can
import cantools
from mu.harness.dir_config import REPO_PARENT_DIR

DBC_PATH = os.path.join(REPO_PARENT_DIR, 'system_can.dbc')
BUS_RECV_TIMEOUT = 0.5

Msg = namedtuple('Msg', ['name', 'data'])


class CanIO:
    def __init__(self):
        self.messages = deque()
        self.db = cantools.database.load_file(DBC_PATH)
        self.bus = can.interface.Bus('vcan0', receive_own_messages=True, bustype='socketcan')
        self.killed = False
        self.listen_thread = threading.Thread(target=self.listener)
        self.listen_thread.start()

    def get_latest_by_name(self, name):
        for msg in self.messages:
            if msg.name == name:
                return msg
        return None

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

    def listener(self):
        while not self.killed:
            raw_msg = self.bus.recv(BUS_RECV_TIMEOUT)
            if raw_msg is None:
                continue
            try:
                msg_data = self.db.decode_message(raw_msg.arbitration_id, raw_msg.data)
                metadata = self.db.get_message_by_frame_id(raw_msg.arbitration_id)
                msg = Msg(metadata.name, msg_data)
                print('[CAN] {}: {}'.format(msg.name, msg.data))
                self.messages.appendleft(msg)
            except KeyError as e:
                print('[CAN] UNKNOWN {}#{}'.format(e, list(raw_msg.data)))
