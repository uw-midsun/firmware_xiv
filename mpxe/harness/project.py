import subprocess
import signal
import threading
import os
import fcntl

from mpxe.harness import decoder
from mpxe.harness.dir_config import REPO_DIR
from mpxe.protogen import stores_pb2

BIN_DIR_FORMAT = os.path.join(REPO_DIR, 'build/bin/x86/{}')

class StoreUpdate:
    def __init__(self, msg, mask, store_type, key):
        self.msg = msg
        self.mask = mask
        self.store_type = store_type
        self.key = key  # use for update stores in sims, init conditions and pm.py


class Project:
    def __init__(self, pm, name, sim):
        self.name = name
        self.pm = pm
        self.killed = False
        self.stores = {}
        self.write_sem = threading.Semaphore(value=0)

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
        # pass a closure with the write to main thread via push_queue
        def write_closure():
            self.popen.stdin.write(msg)
            self.popen.stdin.flush()
            # return the write_sem so main thread can unblock this one post-write
            return self.write_sem
        self.pm.push_queue.put(write_closure)
        # after main thread calls the closure it'll increment the returned sem, unblocking this
        self.write_sem.acquire()

    def handle_store(self, msg):
        store_info = decoder.decode_store_info(msg)
        if store_info.type == stores_pb2.LOG:
            mxlog = stores_pb2.MxLog()
            mxlog.ParseFromString(store_info.msg)
            self.sim.handle_log(self.pm, self, mxlog.log.decode('utf-8').rstrip())
        else:
            key = (store_info.type, store_info.key)
            self.stores[key] = decoder.decode_store(store_info)
            self.sim.handle_update(self.pm, self, key)
