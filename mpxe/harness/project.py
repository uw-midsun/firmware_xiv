import subprocess
import os
import fcntl
import signal
import importlib
import sys

from . import decoder

class Project:
    def __init__(self, name):
        self.name = name
        self.stores = {}
        cmd = 'build/bin/x86/{}'.format(self.name)
        self.popen = subprocess.Popen(cmd, bufsize=0, shell=False, stdin=subprocess.PIPE,
            stdout=subprocess.PIPE, stderr=subprocess.STDOUT, universal_newlines=False)
        ctop_fifo_path = '/tmp/{}_ctop'.format(self.popen.pid)
        while not os.path.exists(ctop_fifo_path):
            pass
        # Subprocess is expected to create child to parent fifo, open in raw bytes mode
        self.ctop_fifo = open(ctop_fifo_path, 'rb')
        if self.ctop_fifo == None:
            raise Exception('failed to open ctop fifo')
        ctop_fd = self.ctop_fifo.fileno()
        ctop_fl = fcntl.fcntl(ctop_fd, fcntl.F_GETFL)
        fcntl.fcntl(ctop_fd, fcntl.F_SETFL, ctop_fl | os.O_NONBLOCK)
        # set up project handler
        importlib.import_module('harness.mocks.' + name)
        handler_class_name = ''.join(s.capitalize() for s in name.split('_'))
        self.handler = getattr(sys.modules['harness.mocks.' + name], handler_class_name)()
        print('started', self.name)
    def stop(self):
        self.popen.kill()
        ctop_fifo_path = '/tmp/{}_ctop'.format(self.popen.pid)
        os.unlink(ctop_fifo_path)
        print('stopped', self.name)
    def write(self, msg):
        self.popen.stdin.write(msg)
        self.popen.stdin.flush()
    def handle_store(self, pm, msg):
        store_info = decoder.decode_store_info(msg)
        key = (store_info.type, store_info.key)
        self.stores[key] = decoder.decode_store(store_info)
        self.handler.handle_update(pm, self)
    def handle_log(self, pm, log):
        self.handler.handle_log(pm, self, log)
