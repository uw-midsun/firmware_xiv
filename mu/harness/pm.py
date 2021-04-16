import os
import threading
import select
import signal
import queue

from mu.harness import canio
from mu.harness import project
from mu.harness.dir_config import REPO_DIR
from mu.sims.sim import Sim
from mu.protogen import stores_pb2

POLL_TIMEOUT = 0.5

# pylint: disable=too-many-instance-attributes
# signals are set in python and C, change in both places if changing
POLL_LOCK_SIGNAL = signal.SIGUSR1
PUSH_LOCK_SIGNAL = signal.SIGUSR2


class InvalidPollError(Exception):
    pass


class ProjectManager:
    def __init__(self):
        # index projects by stdout and ctop_fifo fd
        # ctop_fifo is the child-to-parent fifo created by the C program
        self.fd_to_proj = {}
        self.proj_name_list = os.listdir(os.path.join(REPO_DIR, 'projects'))
        self.killed = False

        # all writes are done on the main thread via the push_queue
        self.push_sem = threading.Semaphore(value=0)
        signal.signal(PUSH_LOCK_SIGNAL, self.signal_push_sem)
        self.push_queue = queue.Queue()
        self.push_thread = threading.Thread(target=self.push)
        self.push_thread.start()

        # run listener threads
        self.poll_thread = threading.Thread(target=self.poll)
        self.poll_thread.start()
        self.can = canio.CanIO()

    def start(self, name, sim=None, init_conds=None):
        if name not in self.proj_name_list:
            raise ValueError('invalid project "{}": expected something from projects directory')
        proj = project.Project(self, name, sim or Sim())
        self.fd_to_proj[proj.popen.stdout.fileno()] = proj

        if init_conds:
            for update in init_conds:
                proj.write_store(update)

        proj.send_command(stores_pb2.MuCmdType.FINISH_INIT_CONDS)
        return proj

    def stop(self, proj):
        del self.fd_to_proj[proj.popen.stdout.fileno()]
        proj.stop()

    def stop_all(self):
        for proj in list(self.fd_to_proj.values()):
            if not proj.killed:
                self.stop(proj)

    def signal_push_sem(self, signum, stack_frame):
        self.push_sem.release()

    def push(self):
        def read_queue():
            write_closure = self.push_queue.get(timeout=0.1)
            proj_write_sem = write_closure()
            # block until the signal handler is called
            self.push_sem.acquire()
            # release the project write sem to unblock its write
            proj_write_sem.release()

        while not self.killed:
            try:
                read_queue()
            except queue.Empty:
                continue

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
            proj.handle_store(msg)
            proj.popen.send_signal(POLL_LOCK_SIGNAL)

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
        self.push_thread.join()
        self.stop_all()
