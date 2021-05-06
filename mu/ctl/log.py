import os

def logs(args):
    if args.sim == '':
        # tail passed twice since execlp passes as argv, where argv[0] should be tail
        os.execlp('tail', 'tail', '-f', '/home/vagrant/shared/firmware_xiv/.mu.log')

    os.execlp('curl', 'curl', '-s', 'http://localhost:8989/logs?sim={}'.format(args.sim))
