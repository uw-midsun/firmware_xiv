import json

import mu.ctl.req as req


def reset(args):
    r = req.send('reset')
    print(r.text)



def start(args):
    params = { 'sim': args.sim, 'proj': args.proj }
    r = req.send('start', params)
    print(r.text)


def stop(args):
    params = { 'sim': args.sim }
    r = req.send('stop', params)
    print(r.text)



def sims(args):
    r = req.send('sims')
    sims = json.loads(r.text)
    print('Available sims:')
    for sim in sims:
        print(sim)


def sim_list(args):
    r = req.send('list')
    sims = json.loads(r.text)
    print('Running sims:')
    for sim in sims:
        print(sim)
