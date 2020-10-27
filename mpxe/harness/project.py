import subprocess
import os
import fcntl
import signal
import importlib
import sys

from mpxe.harness import decoder
from mpxe.protogen import stores_pb2
from mpxe.sims import sim

class Project:
    def __init__(self, name, sim):
        self.name = name
        self.killed = False
        self.stores = {}

        cmd = 'build/bin/x86/{}'.format(self.name)
        print('command', cmd)
        self.popen = subprocess.Popen(cmd, bufsize=0, shell=False, stdin=subprocess.PIPE,
            stdout=subprocess.PIPE, stderr=subprocess.STDOUT, universal_newlines=False)
        ctop_fifo_path = '/tmp/{}_ctop'.format(self.popen.pid)
        print('ctop_fifo_path', ctop_fifo_path)
        while not os.path.exists(ctop_fifo_path):
            pass

        # Subprocess is expected to create child to parent fifo, open in raw bytes mode
        self.ctop_fifo = open(ctop_fifo_path, 'rb')
        if self.ctop_fifo is None:
            raise Exception('failed to open ctop fifo')
        ctop_fd = self.ctop_fifo.fileno()
        ctop_fl = fcntl.fcntl(ctop_fd, fcntl.F_GETFL)
        fcntl.fcntl(ctop_fd, fcntl.F_SETFL, ctop_fl | os.O_NONBLOCK)

        self.sim = sim
        print('started', self.name)

    def stop(self):
        if self.killed:
            return
        self.popen.terminate()
        self.popen.wait()
        self.popen.stdout.close()
        self.popen.stdin.close()
        self.ctop_fifo.close()
        ctop_fifo_path = '/tmp/{}_ctop'.format(self.popen.pid)
        os.unlink(ctop_fifo_path)
        self.killed = True
        print('stopped', self.name)

    def write_store(self, store_type, msg, mask):
        update = stores_pb2.MxStoreUpdate()
        update.key = 0
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
