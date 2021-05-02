import json

import mu.ctl.req as req
from mu.srv.router import get_routes

def reset(args):
    r = req.send(get_routes()['reset'])
    print(r)

def start(args):
    body = { 'sim': args.sim, 'proj': args.proj }
    r = req.send(get_routes()['sim_start'], body)
    print(r)

def stop(args):
    body = { 'sim': args.sim }
    r = req.send(get_routes()['sim_stop'], body)
    print(r)

def catalog(args):
    r = req.send(get_routes()['sim_cat'])
    sims = json.loads(r)
    for sim in sims:
        print(sim)

def sim_list(args):
    r = req.send(get_routes()['sim_list'])
    fd_sims = json.loads(r)
    for pair in fd_sims:
        print(pair['fd'], pair['sim'])
