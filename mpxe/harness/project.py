import subprocess
import os
import fcntl

from mpxe.harness import decoder
from mpxe.harness.dir_config import REPO_DIR
from mpxe.protogen import stores_pb2

BIN_DIR_FORMAT = os.path.join(REPO_DIR, 'build/bin/x86/{}')


class Project:
    def __init__(self, name, sim):
        self.name = name
        self.killed = False
        self.stores = {}

        cmd = BIN_DIR_FORMAT.format(self.name)
        self.popen = subprocess.Popen(cmd, bufsize=0, shell=False, stdin=subprocess.PIPE,
                                      stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                      universal_newlines=False)

        # Set the flag O_NONBLOCK on stdout, necessary for reading from running projects
        flags = fcntl.fcntl(self.popen.stdout.fileno(), fcntl.F_GETFL)
        fcntl.fcntl(self.popen.stdout.fileno(), fcntl.F_SETFL, flags | os.O_NONBLOCK)

        self.sim = sim

    def stop(self):
        if self.killed:
            return
        self.popen.terminate()
        self.popen.wait()
        self.popen.stdout.close()
        self.popen.stdin.close()
        self.killed = True

    def write_store(self, msg, mask, store_type, key=0):
        update = stores_pb2.MxStoreUpdate()
        update.key = key
        update.type = store_type
        update.msg = msg.SerializeToString()
        update.mask = mask.SerializeToString()
        self.write(update.SerializeToString())

    def write(self, msg):
        self.popen.stdin.write(msg)
        self.popen.stdin.flush()

    def handle_store(self, pm, msg):
        store_info = decoder.decode_store_info(msg)
        if store_info.type == stores_pb2.LOG:
            mxlog = stores_pb2.MxLog()
            mxlog.ParseFromString(store_info.msg)
            self.sim.handle_log(pm, self, mxlog.log.decode('utf-8').rstrip())
        else:
            key = (store_info.type, store_info.key)
            self.stores[key] = decoder.decode_store(store_info)
            self.sim.handle_update(pm, self)
