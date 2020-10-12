import os
import threading
import select
import subprocess
import signal

from . import project
from . import canio

class ProjectManager:
    def __init__(self):
        # index projects by stdout and ctop_fifo fd
        self.proj_fds = {}
        self.statuses = {}
        self.proj_name_list = os.listdir('/home/vagrant/shared/firmware_xiv/projects')
        self.killed = False
        # run listener threads
        self.poll_thread = threading.Thread(target=self.poll)
        self.poll_thread.start()
        self.can = canio.Canio()

    # DEPRECATED
    def build(self, name):
        # check if already built or project doesn't exist
        if name not in self.proj_name_list:
            raise Exception('invalid project')
        cmd = 'make build PROJECT={} PIECE= PLATFORM=x86 DEFINE=MPXE; exit'.format(name)
        print('[make] making', name)
        p = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, universal_newlines=True)
        # block while making
        for line in iter(p.stdout.readline, ''):
            print('[make]', line.rstrip())
        p.wait()
        print('[make]', name, 'exited with code', p.returncode)
        if p.returncode != 0:
            raise Exception('build failed')
        self.statuses[name] = True
        return True
        
    def start(self, name):
        if name not in self.proj_name_list:
            raise Exception('invalid project')
        proj = project.Project(name)
        self.proj_fds[proj.ctop_fifo.fileno()] = proj
        self.proj_fds[proj.popen.stdout.fileno()] = proj
        return proj

    def stop(self, proj):
        proj.stop()
        del self.proj_fds[proj.ctop_fifo.fileno()]
        del self.proj_fds[proj.popen.stdout.fileno()]
    
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
                proj.handle_log(self, s.decode('utf-8'))
            elif res[0] == proj.ctop_fifo.fileno():
                # Currently assume all messages are storeinfo,
                # will need other message types
                msg = proj.ctop_fifo.read()
                proj.handle_store(self, msg)
                proj.popen.send_signal(signal.SIGUSR1)

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
