import os
from enum import Enum
import sys
from time import sleep
import threading
import select
import subprocess

proj_fd = {}
proj_id = {}

statuses = {}

def del_proj(pid=None, fd=None):
    global proj_fd
    global proj_id
    if pid != None:
        p = proj_id[pid]
        del proj_id[pid]
        del proj_fd[p.stdout.fileno()]
    if fd != None:
        p = proj_fd[fd]
        del proj_fd[fd]
        del proj_id[p.pid]

def add_proj(p):
    global proj_fd
    global proj_id
    proj_fd[p.stdout.fileno()] = p
    proj_id[p.pid] = p

def poll_thread():
    global proj_fd
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
            proj = proj_fd[res[0]]
            if res[1] == select.POLLIN:
                s = proj.stdout.readline().rstrip()
                print('[{}]'.format(proj.pid), s)
            elif res[1] == select.POLLHUP:
                del_proj(proj.pid)

def popen(cmd):
    p = subprocess.Popen(cmd, shell=True, stdin=subprocess.PIPE,
                         stdout=subprocess.PIPE, universal_newlines=True)
    return p

def run(name):
    built = statuses[name]
    if built is None or built is False:
        return False
    cmd = 'build/bin/x86/{}'.format(name)
    print('starting', name)
    p = popen(cmd)
    add_proj(p)

def build(name):
    global statuses
    if statuses[name] is None or statuses[name] is True:
        return False
    cmd = 'make build PROJECT={} PIECE= PLATFORM=x86 DEFINE=MPXE; exit'.format(name)
    p = popen(cmd)
    for line in iter(p.stdout.readline, ''):
        print('make', line.rstrip())
    p.wait()
    print('make', name, 'exited with code', p.returncode)
    if p.returncode != 0:
        return False
    statuses[name] = True
    return True

def init():
    global statuses
    proj_name_list = os.listdir('/home/vagrant/shared/firmware_xiv/projects')
    for name in proj_name_list:
        statuses[name] = False
    for name in os.listdir('/home/vagrant/shared/firmware_xiv/build/bin/x86'):
        statuses[name] = True
    build('can_communication')
    build('can_dump')
    pt = threading.Thread(target=poll_thread)
    pt.start()
    run('can_communication')
    run('can_dump')
    pt.join()