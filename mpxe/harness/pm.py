import os
import threading
import select
import subprocess
import project

class ProjectManager:
    def __init__(self):
        self.proj_fds = {}
        self.statuses = {}
        proj_name_list = os.listdir('/home/vagrant/shared/firmware_xiv/projects')
        for name in proj_name_list:
            self.statuses[name] = False
        self.poll_thread = threading.Thread(target=self.poll)
        self.poll_thread.start()
    def build(self, name):
        # check if already built or project doesn't exist
        if self.statuses[name] is None or self.statuses[name] is True:
            raise Exception('invalid project')
        cmd = 'make build PROJECT={} PIECE= PLATFORM=x86 DEFINE=MPXE; exit'.format(name)
        print('making', name)
        p = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, universal_newlines=True)
        for line in iter(p.stdout.readline, ''):
            print('make', line.rstrip())
        p.wait()
        print('make', name, 'exited with code', p.returncode)
        if p.returncode != 0:
            raise Exception('build failed')
        self.statuses[name] = True
        return True
    def start(self, name):
        if name not in self.statuses:
            raise Exception('invalid project')
        if self.statuses[name] is False:
            raise Exception('unbuilt project')
        proj = project.Project(name)
        self.proj_fds[proj.ctop_fifo.fileno()] = proj
        self.proj_fds[proj.popen.stdout.fileno()] = proj
        return proj.ctop_fifo.fileno()
    def stop(self, fd):
        p = self.proj_fds[fd]
        p.stop()
        del self.proj_fds[p.ctop_fifo.fileno()]
        del self.proj_fds[p.popen.stdout.fileno()]
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
                # Just print logs for now
                s = proj.popen.stdout.readline().rstrip()
                print('[{}]'.format(proj.popen.pid), s.decode('utf-8'))
            elif res[0] == proj.ctop_fifo.fileno():
                # Currently assume all messages are storeinfo,
                # will need other message types
                msg = proj.ctop_fifo.read()
                proj.handle_store(self, msg)
        try:
            while True:
                if len(self.proj_fds) == 0:
                    continue
                p = prep_poll()
                res_list = p.poll()
                for res in res_list:
                    handle_poll_res(res)
        except Exception as e:
            if str(e) == 'done':
                return
            raise e
    def end(self):
        self.poll_thread.join()
