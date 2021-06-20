import os
import threading
import select
import signal
import queue
import importlib
import inspect

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
            config = Config(False, 'vcan0', False)
        print('Configuration:', config)
        self.config = config
        self.fd_to_sim = {}
        self.proj_name_list = os.listdir(os.path.join(REPO_DIR, 'projects'))
        self.sim_catalog = self.sim_cat()
        self.killed = False

        # all writes are done on the main thread via the push_queue
        self.push_sem = threading.Semaphore(value=0)
        try:
            signal.signal(PUSH_LOCK_SIGNAL, self.signal_push_sem)
        except ValueError:
            pass
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

    def reset(self):
        config = self.config
        self.end()
        self.__init__(config=config)

    def start(self, sim_class, proj_name=''):
        if isinstance(sim_class, str):
            try:
                sim_class = self.sim_catalog[sim_class]
            except KeyError as e:
                raise ValueError('Invalid sim, check catalog') from e
        if proj_name:
            if proj_name not in self.proj_name_list:
                raise ValueError('invalid project "{}": expected something from projects directory')
            return sim_class(self, proj_name=proj_name)
        return sim_class(self)

    def register(self, sim):
        self.fd_to_sim[sim.fd] = sim

    def stop(self, sim):
        del self.fd_to_sim[sim.fd]
        sim.stop()

    def stop_name(self, sim_name):
        for sim in self.fd_to_sim.values():
            if sim.__class__.__name__ == sim_name:
                self.stop(sim)
                return
        raise ValueError('Invalid sim, check list')

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
        if not self.config.projlogs:
            return
        sub = logger.Subscriber('pm')
        self.logger.subscribe(sub)
        while not self.killed:
            try:
                log = sub.get()
                print('[{}] {}'.format(log.tag, log.msg), flush=True)
            except logger.NoLog:
                continue
        self.logger.unsubscribe(sub)

    def sim_list(self):
        ret = []
        for fd in self.fd_to_sim:
            ret.append(self.fd_to_sim[fd].__class__.__name__)
        return ret

    def sim_cat(self):
        sim_files = os.listdir(os.path.join(REPO_DIR, 'mu', 'sims'))
        catalog = {}
        for sim_file in sim_files:
            if not sim_file.endswith('.py'):
                continue
            sim_name = sim_file[:len(sim_file) - 3]
            mod_name = 'mu.sims.{}'.format(sim_name)
            mod = importlib.import_module(mod_name)
            sim_classes = inspect.getmembers(mod, inspect.isclass)
            for sim_name, sim_class in sim_classes:
                if BoardSim not in inspect.getmro(sim_class) or sim_name in catalog:
                    continue
                catalog[sim_name] = sim_class
        return catalog

    def end(self):
        self.killed = True
        self.can.stop()
        self.poll_thread.join()
        self.push_thread.join()
        self.stop_all()
