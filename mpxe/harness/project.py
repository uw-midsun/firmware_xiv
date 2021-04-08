import subprocess
import signal
import threading
import os
import fcntl
import time

from mpxe.harness import decoder
from mpxe.harness.dir_config import REPO_DIR
from mpxe.protogen import stores_pb2

BIN_DIR_FORMAT = os.path.join(REPO_DIR, 'build/bin/x86/{}')
INIT_LOCK_SIGNAL = signal.SIGUSR2


class StoreUpdate:
    def __init__(self, msg, mask, store_type, key):
        self.msg = msg
        self.mask = mask
        self.store_type = store_type
        self.key = key  # use for update stores in sims, init conditions and pm.py


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

        # set up initialization lock for STDIN
        self.init_lock = threading.Lock()
        signal.signal(INIT_LOCK_SIGNAL, self.init_lock_signal)
        self.sim = sim

    def init_lock_signal(self, signum, stack_frame):
        self.init_lock.release()

    def stop(self):
        if self.killed:
            return
        self.popen.terminate()
        self.popen.wait()
        self.popen.stdout.close()
        self.popen.stdin.close()
        self.killed = True

    def write_store(self, store_update):
        update = stores_pb2.MxStoreUpdate()
        update.key = store_update.key
        update.type = store_update.store_type
        update.msg = store_update.msg.SerializeToString()
        update.mask = store_update.mask.SerializeToString()
        self.write(update.SerializeToString())

    def send_command(self, cmd):
        mxcmd_msg = stores_pb2.MxCmd()
        mxcmd_msg.cmd = cmd
        update = stores_pb2.MxStoreUpdate()
        update.type = stores_pb2.MxStoreType.CMD
        update.msg = mxcmd_msg.SerializeToString()
        self.write(update.SerializeToString())

    def write(self, msg):
        # lock for next write store call received
        self.init_lock.acquire()
        self.popen.stdin.write(msg)
        self.popen.stdin.flush()
        # Block until C sends signal that data has been read
        self.init_lock.acquire()
        self.init_lock.release()

    def handle_store(self, pm, msg):
        store_info = decoder.decode_store_info(msg)
        if store_info.type == stores_pb2.LOG:
            mxlog = stores_pb2.MxLog()
            mxlog.ParseFromString(store_info.msg)
            self.sim.handle_log(pm, self, mxlog.log.decode('utf-8').rstrip())
        else:
            key = (store_info.type, store_info.key)
            self.stores[key] = decoder.decode_store(store_info)
            self.sim.handle_update(pm, self, key)
