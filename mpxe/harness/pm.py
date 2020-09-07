import os
from enum import Enum
import sys
from time import sleep
import threading
import select
import fcntl
import subprocess
import ds

# Two dictionaries to index by both fd and pid
# Pid indexing may not actually be useful.
proj_fd = {}
proj_id = {}

statuses = {}

class Proj:
    def __init__(self, name: str):
        self.name = name
        self.popen = None
        self.ctop_fifo = None

    def run(self):
        # Don't run if it isn't built yet
        built = statuses[self.name]
        if built is None or built is False:
            return False
        cmd = 'build/bin/x86/{}'.format(self.name)
        self.popen = subprocess.Popen(cmd, bufsize=0, shell=False, stdin=subprocess.PIPE,
            stdout=subprocess.PIPE, stderr=subprocess.STDOUT, universal_newlines=False)
        # Subprocess is expected to create child to parent fifo, open in raw bytes mode
        self.ctop_fifo = open('/tmp/{}_ctop'.format(self.popen.pid), 'rb')
        # Flag setting may not be necessary
        ctop_fd = self.ctop_fifo.fileno()
        ctop_fl = fcntl.fcntl(ctop_fd, fcntl.F_GETFL)
        fcntl.fcntl(ctop_fd, fcntl.F_SETFL, ctop_fl | os.O_NONBLOCK)
        add_proj(self)
        print('started', self.name)

    def stop(self):
        self.popen.kill()
        close(self.ctop_fifo)
        os.unlink('/tmp/{}_ctop'.format(self.popen.pid))
        del_proj(pid=self.popen.pid)

    def write(self, msg):
        self.popen.stdin.write(msg)
        self.popen.stdin.flush()

# Remove proj from global dictionaries
def del_proj(pid=None, fd=None):
    global proj_fd
    global proj_id
    if pid != None:
        p = proj_id[pid]
        del proj_id[pid]
        del proj_fd[p.popen.stdout.fileno()]
        del proj_fd[p.ctop_fifo.fileno()]
    if fd != None:
        p = proj_fd[fd]
        del proj_fd[fd]
        del proj_id[p.popen.pid]
        del proj_fd[p.ctop_fifo.fileno()]

# Add proj to global dictionaries
# Also index by child to parent fifo fd so
# that we can poll for everything at once
def add_proj(p):
    global proj_id
    global proj_fd
    proj_id[p.popen.pid] = p
    proj_fd[p.popen.stdout.fileno()] = p
    proj_fd[p.ctop_fifo.fileno()] = p

def poll_thread():
    global proj_fd
    # helper function to register all fds, stdout and ctop
    def prep_poll():
        poll = select.poll()
        for fd in proj_fd.keys():
            poll.register(fd, select.POLLIN)
        return poll
    while True:
        if len(proj_fd) == 0:
            continue
        p = prep_poll()
        res_list = p.poll()
        for res in res_list:
            # res is tuple (fd, event)
            proj = proj_fd[res[0]]
            # only act on POLLIN events
            if res[1] == select.POLLIN:
                # Check if we should check stdout or ctop
                if res[0] == proj.popen.stdout.fileno():
                    # Just print logs for now
                    s = proj.popen.stdout.readline().rstrip()
                    print('[{}]'.format(proj.popen.pid), s.decode('utf-8'))
                elif res[0] == proj.ctop_fifo.fileno():
                    # Currently assume all messages are storeinfo,
                    # will need other message types
                    msg = proj.ctop_fifo.read()
                    ds.handle_store(msg)
            elif res[1] == select.POLLHUP:
                # TODO: improve close handling
                del_proj(proj.popen.pid)

# Blocks while executing. Just runs the make command and raises exception if failed.
def build(name):
    global statuses
    if statuses[name] is None or statuses[name] is True:
        return False
    cmd = 'make build PROJECT={} PIECE= PLATFORM=x86 DEFINE=MPXE; exit'.format(name)
    p = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, universal_newlines=True)
    for line in iter(p.stdout.readline, ''):
        print('make', line.rstrip())
    p.wait()
    print('make', name, 'exited with code', p.returncode)
    if p.returncode != 0:
        raise Exception('build failed')
        return False
    statuses[name] = True
    return True

def init():
    global statuses
    proj_name_list = os.listdir('/home/vagrant/shared/firmware_xiv/projects')
    for name in proj_name_list:
        statuses[name] = False
    # For now just call directly. Eventually, provide a library interface.
    build('controller_board_blinking_leds')
    pt = threading.Thread(target=poll_thread)
    pt.start()
    p = Proj('controller_board_blinking_leds')
    p.run()
    # TODO: properly handle interrupts rather than calling join
    pt.join()
