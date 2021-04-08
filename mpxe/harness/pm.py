import os
import threading
import select
import signal
from time import sleep

from mpxe.harness import canio
from mpxe.harness import project
from mpxe.harness.dir_config import REPO_DIR
from mpxe.sims.sim import Sim
from mpxe.protogen import stores_pb2

POLL_TIMEOUT = 0.5

# signals are set in python and C, change in both places if changing
STORE_LOCK_SIGNAL = signal.SIGUSR1


class InvalidPollError(Exception):
    pass


class ProjectManager:
    def __init__(self):
        # index projects by stdout and ctop_fifo fd
        # ctop_fifo is the child-to-parent fifo created by the C program
        self.fd_to_proj = {}
        self.proj_name_list = os.listdir(os.path.join(REPO_DIR, 'projects'))
        self.killed = False

        # run listener threads
        self.poll_thread = threading.Thread(target=self.poll)
        self.poll_thread.start()
        self.can = canio.CanIO()

    def start(self, name, sim=None, init_conds=None):
        if name not in self.proj_name_list:
            raise ValueError('invalid project "{}": expected something from projects directory')
        proj = project.Project(name, sim or Sim())
        self.fd_to_proj[proj.popen.stdout.fileno()] = proj

        if init_conds:
            for update in init_conds:
                proj.write_store(update)

        proj.send_command(stores_pb2.MxCmdType.FINISH_INIT_CONDS)
        return proj

    def stop(self, proj):
        del self.fd_to_proj[proj.popen.stdout.fileno()]
        proj.stop()

    def stop_all(self):
        for proj in list(self.fd_to_proj.values()):
            if not proj.killed:
                self.stop(proj)

    def poll(self):
        def prep_poll():
            poll = select.poll()
            for fd in self.fd_to_proj:
                poll.register(fd, select.POLLIN)
            return poll

        def handle_poll_res(fd, event):
            if (event & select.POLLIN) == 0:
                raise InvalidPollError
            proj = self.fd_to_proj[fd]
            # Currently assume all messages are storeinfo,
            # might need other message types
            msg = proj.popen.stdout.read()
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
        except InvalidPollError:
            return

    def end(self):
        self.killed = True
        self.can.stop()
        self.poll_thread.join()
        self.stop_all()
