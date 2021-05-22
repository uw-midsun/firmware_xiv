import os

from mu.srv.server import TCP_PORT

def logs(args):
    if args.sim == '':
        # tail passed twice since execlp passes as argv, where argv[0] should be tail
        os.execlp('tail', 'tail', '-f', '/home/vagrant/shared/firmware_xiv/.mu.log')

    os.execlp('curl', 'curl', '-s', 'http://localhost:{}/logs?sim={}'.format(TCP_PORT, args.sim))
