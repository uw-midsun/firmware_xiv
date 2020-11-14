import os
import threading
import select
import subprocess
import signal

REPO_ROOT_DIR = '/home/vagrant/shared/'

from mpxe.harness import project
from mpxe.harness import canio
from mpxe.sims.sim import Sim

POLL_TIMEOUT = 0.5

# signals are set in python and C, change in both places if changing
STORE_LOCK_SIGNAL = signal.SIGUSR1
LOG_LOCK_SIGNAL = signal.SIGUSR2

class InvalidPollError(Exception):
    pass

class ProjectManager:
    def __init__(self):
        # index projects by stdout and ctop_fifo fd
        # ctop_fifo is the child-to-parent fifo created by the C program
        self.fd_to_proj = {}
        self.proj_name_list = os.listdir(REPO_ROOT_DIR + 'firmware_xiv/projects')
        self.killed = False
        # run listener threads
        self.poll_thread = threading.Thread(target=self.poll)
        self.poll_thread.start()
        self.can = canio.CanIO()
        
    def start(self, name, sim=None):
        if name not in self.proj_name_list:
            raise ValueError('invalid project "{}": expected something from projects directory')
        proj = project.Project(name, sim or Sim())
        self.fd_to_proj[proj.ctop_fifo.fileno()] = proj
        self.fd_to_proj[proj.popen.stdout.fileno()] = proj
        return proj

    def stop(self, proj):
        del self.fd_to_proj[proj.ctop_fifo.fileno()]
        del self.fd_to_proj[proj.popen.stdout.fileno()]
        proj.stop()

    def stop_all(self):
        for proj in list(self.fd_to_proj.values()):
            if not proj.killed:
                self.stop(proj)

    def poll(self):
        def prep_poll():
            poll = select.poll()
            for fd in self.fd_to_proj.keys():
                poll.register(fd, select.POLLIN)
            return poll

        def handle_poll_res(fd, event):
            if not (event & select.POLLIN):
                raise InvalidPollError
            proj = self.fd_to_proj[fd]
            # Check if we should check stdout or ctop
            if fd == proj.popen.stdout.fileno():
                s = proj.popen.stdout.readline().rstrip()
                proj.handle_log(self, s.decode('utf-8'))
                proj.popen.send_signal(LOG_LOCK_SIGNAL)
            elif fd == proj.ctop_fifo.fileno():
                # Currently assume all messages are storeinfo,
                # will need other message types
                msg = proj.ctop_fifo.read()
                proj.handle_store(self, msg)
                proj.popen.send_signal(STORE_LOCK_SIGNAL)

        try:
            while not self.killed:
                if not self.fd_to_proj:
                    continue
                p = prep_poll()
                res_list = p.poll(POLL_TIMEOUT)
                for fd, event in res_list:
                    handle_poll_res(fd, event)
        except InvalidPollError as e:
            return

    def end(self):
        self.killed = True
        self.can.stop()
        self.poll_thread.join()
        self.stop_all()
