import os
import threading
import select
import signal
import queue

from mu.harness import canio
from mu.harness.dir_config import REPO_DIR
from mu.harness import decoder
from mu.harness.board_sim import BoardSim
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
        self.can = canio.CanIO()

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
        for sim in list(self.fd_to_sim.values()):
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
            # Currently assume all messages are storeinfo,
            # might need other message types
            msg = sim.proj.popen.stdout.read()
            store_info = decoder.decode_store_info(msg)
            if store_info.type == stores_pb2.LOG:
                mulog = stores_pb2.MuLog()
                mulog.ParseFromString(store_info.msg)
                log = mulog.log.decode().rstrip()
                print(log)
            else:
                sim.handle_info(store_info)
            sim.proj.popen.send_signal(POLL_LOCK_SIGNAL)

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

    def end(self):
        self.killed = True
        self.can.stop()
        self.poll_thread.join()
        self.push_thread.join()
        self.stop_all()
