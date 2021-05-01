import os
import threading
import select
import signal
import queue

from mu.srv.config import Config
from mu.harness import canio
from mu.harness import logger
from mu.harness.dir_config import REPO_DIR
from mu.harness.board_sim import BoardSim

POLL_TIMEOUT = 0.5

# pylint: disable=too-many-instance-attributes
# signals are set in python and C, change in both places if changing
PUSH_LOCK_SIGNAL = signal.SIGUSR2


class InvalidPollError(Exception):
    pass


class ProjectManager:
    def __init__(self, config=None):
        if not config:
            config = Config(False, 'vcan0')
        self.config = config
        self.fd_to_sim = {}
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

        # Can initialization can fail with an invalid canbus, so cleanup is necessary
        try:
            self.can = canio.CanIO(self, bus_name=config.canbus)
        except OSError:
            self.end()
            raise

        # setup logging
        self.logger = logger.Logger()
        self.log_thread = threading.Thread(target=self.log_all)
        self.log_thread.start()

    def start(self, proj_name, sim_class=BoardSim):
        if proj_name not in self.proj_name_list:
            raise ValueError('invalid project "{}": expected something from projects directory')
        sim = sim_class(self, proj_name)
        return sim

    def register(self, sim):
        self.fd_to_sim[sim.fd] = sim

    def stop(self, sim):
        del self.fd_to_sim[sim.fd]
        sim.stop()

    def stop_all(self):
        for sim in self.fd_to_sim.values():
            sim.stop()

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
            for fd in self.fd_to_sim:
                poll.register(fd, select.POLLIN)
            return poll

        def handle_poll_res(fd, event):
            if (event & select.POLLIN) == 0:
                raise InvalidPollError
            sim = self.fd_to_sim[fd]
            sim.process_pipe()

        try:
            while not self.killed:
                if not self.fd_to_sim:
                    continue
                p = prep_poll()
                res_list = p.poll(POLL_TIMEOUT)
                for fd, event in res_list:
                    handle_poll_res(fd, event)
        except InvalidPollError:
            return

    def log_all(self):
        sub = logger.Subscriber('pm')
        self.logger.subscribe(sub)
        while not self.killed:
            try:
                log = sub.get()
                print('[{}] {}'.format(log.tag, log.msg), flush=True)
            except logger.NoLog:
                continue
        self.logger.unsubscribe(sub)

    def end(self):
        self.killed = True
        self.can.stop()
        self.poll_thread.join()
        self.push_thread.join()
        self.stop_all()
