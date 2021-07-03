import json

import mu.ctl.req as req


def reset(args):
    r = req.send('reset')
    print(r)


def start(args):
    body = {'sim': args.sim, 'proj': args.proj}
    r = req.send('start', body)
    print(r)


def stop(args):
    body = {'sim': args.sim}
    r = req.send('stop', body)
    print(r)


def sims(args):
    r = req.send('sims')
    sims = json.loads(r)
    print('Available sims:')
    for sim in sims:
        print(sim)


def sim_list(args):
    r = req.send('list')
    sims = json.loads(r)
    print('Running sims:')
    for sim in sims:
        print(sim)
