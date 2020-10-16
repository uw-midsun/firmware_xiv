import os
import threading
import select
import subprocess

from mpxe.harness import project
from mpxe.harness import canio
from mpxe.harness import decoder
from mpxe.sims.sim import Sim

class ProjectManager:
    def __init__(self, can_bus_name='vcan0'):
        # index projects by stdout and ctop_fifo fd
        self.proj_fds = {}
        self.statuses = {}
        self.log_callbacks = []
        self.store_callbacks = []
        self.can_callbacks = []
        self.proj_name_list = os.listdir('/home/vagrant/shared/firmware_xiv/projects')
        self.killed = False
        # run listener threads
        self.poll_thread = threading.Thread(target=self.poll)
        self.poll_thread.start()
        self.can = canio.Canio(can_bus_name)
        
    def start(self, name, sim=None):
        if name not in self.proj_name_list:
            raise Exception('invalid project')
        proj = project.Project(name, sim or Sim())
        self.log_callbacks.append(proj.handle_log)
        self.store_callbacks.append(proj.handle_store)
        self.proj_fds[proj.ctop_fifo.fileno()] = proj
        self.proj_fds[proj.popen.stdout.fileno()] = proj
        return proj

    def stop(self, proj):
        del self.proj_fds[proj.ctop_fifo.fileno()]
        del self.proj_fds[proj.popen.stdout.fileno()]
        proj.stop()
    
    def stop_all(self):
        for proj in list(self.proj_fds.values()):
            if not proj.killed:
                self.stop(proj)

    def poll(self):
        def prep_poll():
            poll = select.poll()
            for fd in self.proj_fds.keys():
                poll.register(fd, select.POLLIN)
            return poll

        def handle_poll_res(res):
            if res[1] != select.POLLIN:
                raise Exception('done')
            # res is tuple (fd, event)
            proj = self.proj_fds[res[0]]
            # Check if we should check stdout or ctop
            if res[0] == proj.popen.stdout.fileno():
                s = proj.popen.stdout.readline().rstrip()
                decoded_log = s.decode('utf-8')
                for cb in self.log_callbacks:
                    cb(proj, self, decoded_log)
            elif res[0] == proj.ctop_fifo.fileno():
                # Currently assume all messages are storeinfo,
                # will need other message types
                msg = proj.ctop_fifo.read()
                for cb in self.store_callbacks:
                    store_info = decoder.decode_store_info(msg)
                    cb(proj, self, store_info)

        try:
            while not self.killed:
                if len(self.proj_fds) == 0:
                    continue
                p = prep_poll()
                res_list = p.poll(0.5)
                for res in res_list:
                    handle_poll_res(res)
        except Exception as e:
            if str(e) == 'done':
                return
            raise e

    def end(self):
        self.killed = True
        self.can.stop()
        self.poll_thread.join()
        self.stop_all()
