import subprocess
import threading
import os
import fcntl

from mu.harness.dir_config import REPO_DIR
from mu.protogen import stores_pb2

BIN_DIR_FORMAT = os.path.join(REPO_DIR, 'build/bin/x86/{}')


class StoreUpdate:
    def __init__(self, msg, mask, key):
        # Key is the tuple (store_type, store_key)
        self.msg = msg
        self.mask = mask
        self.key = key


class Project:
    def __init__(self, pm, sim, name):
        self.pm = pm
        self.sim = sim
        self.name = name
        self.killed = False
        self.write_sem = threading.Semaphore(value=0)

        cmd = BIN_DIR_FORMAT.format(self.name)
        self.popen = subprocess.Popen(cmd, bufsize=0, shell=False, stdin=subprocess.PIPE,
                                      stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                      universal_newlines=False)

        # Set the flag O_NONBLOCK on stdout, necessary for reading from running projects
        flags = fcntl.fcntl(self.popen.stdout.fileno(), fcntl.F_GETFL)
        fcntl.fcntl(self.popen.stdout.fileno(), fcntl.F_SETFL, flags | os.O_NONBLOCK)

    def stop(self):
        if self.killed:
            return
        self.popen.terminate()
        self.popen.wait()
        self.popen.stdout.close()
        self.popen.stdin.close()
        self.killed = True

    def write_store(self, store_update):
        update = stores_pb2.MuStoreUpdate()
        update.type = store_update.key[0]
        update.key = store_update.key[1]
        update.msg = store_update.msg.SerializeToString()
        update.mask = store_update.mask.SerializeToString()
        self.write(update.SerializeToString())

    def send_command(self, cmd):
        mucmd_msg = stores_pb2.MuCmd()
        mucmd_msg.cmd = cmd
        update = stores_pb2.MuStoreUpdate()
        update.type = stores_pb2.MuStoreType.CMD
        update.msg = mucmd_msg.SerializeToString()
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
