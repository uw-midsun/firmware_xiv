import os

def logs(args):
    # tail passed twice since execlp passes as argv, where argv[0] should be tail
    os.execlp('tail', 'tail', '-f', '/home/vagrant/shared/firmware_xiv/.mu.log')
