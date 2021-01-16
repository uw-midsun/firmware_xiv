import subprocess
import threading
import os
import fcntl
import signal
import importlib
import sys

from mpxe.harness import decoder
from mpxe.protogen import stores_pb2
from mpxe.harness import pm

BIN_DIR_FORMAT = pm.REPO_ROOT_DIR + 'firmware_xiv/build/bin/x86/{}'

class Project:
    def __init__(self, name, sim):
        self.name = name
        self.killed = False
        self.stores = {}
        self.lock = threading.Lock()

        # Handler for SIGUSR2 sent by manager upon completion of sending startup states
        def sig_handler_unlock(signum, stack_frame):
            self.lock.release()

        signal.signal(signal.SIGUSR2, sig_handler_unlock) # Initialize handler

        cmd = BIN_DIR_FORMAT.format(self.name)
        self.popen = subprocess.Popen(cmd, bufsize=0, shell=False, stdin=subprocess.PIPE,
            stdout=subprocess.PIPE, stderr=subprocess.STDOUT, universal_newlines=False)
        self.ctop_fifo_path = '/tmp/{}_ctop'.format(self.popen.pid)
        while not os.path.exists(self.ctop_fifo_path):
            pass

        # Subprocess is expected to create child to parent fifo, open in raw bytes mode
        self.ctop_fifo = open(self.ctop_fifo_path, 'rb')
        if self.ctop_fifo is None:
            raise IOError('failed to open ctop fifo')

        # Set the flag O_NONBLOCK on the ctop-fifo fd, necessary for reading from running projects
        ctop_fd = self.ctop_fifo.fileno()
        ctop_fl = fcntl.fcntl(ctop_fd, fcntl.F_GETFL)
        fcntl.fcntl(ctop_fd, fcntl.F_SETFL, ctop_fl | os.O_NONBLOCK)

        # Lock fifo so manager can send startup-state messages 
        self.lock.acquire()
        with lock:
            pass # waits for lock to be unlocked (on acquire call), then immediately unlocks it
        
        self.sim = sim

    def stop(self):
        if self.killed:
            return
        self.popen.terminate()
        self.popen.wait()
        self.popen.stdout.close()
        self.popen.stdin.close()
        self.ctop_fifo.close()
        os.unlink(self.ctop_fifo_path)
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
        key = (store_info.type, store_info.key)
        self.stores[key] = decoder.decode_store(store_info)
        self.sim.handle_update(pm, self)

    def handle_log(self, pm, log):
        self.sim.handle_log(pm, self, log)
