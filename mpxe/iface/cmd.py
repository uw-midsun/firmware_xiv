import os
import sys
import inspect
import json
import asyncio

from mpxe.iface import mpxei
from mpxe import sims

class CmdRouter:
    def __init__(self):
        self.routes = {}
    def route(self, route_str):
        def decorator(f):
            self.routes[route_str] = f
            return f
        return decorator
    def handle(self, mpxei, cmd):
        handler = self.routes.get(cmd['cmd'])
        if handler:
            return handler(mpxei, cmd['args'])
        else:
            raise ValueError('Unknown command')

cr = CmdRouter()

@cr.route('init')
def init_handler(mpxei, args):
    print('handling init, args:', args)
    await mpxei.update_q.put({
        'update_type': 'init_ret', 'data': {
            'sims': mpxei.pm.sim_classes.key(),
            'projs': mpxei.pm.proj_name_list,
            'can_msgs': mpxei.can.msg_dict
        }
    })

@cr.route('start')
def start_handler(mpxei, args):
    print('handling start, args:', args)
    proj_name = args['name']
    sim_name = args['sim']
    sim = mpxei.pm.sim_classes['sim_name']()
    proj = mpxei.pm.start('proj_name', sim)
    proj.log_callbacks.append(mpxei.log_callback)
    proj.store_callbacks.append(mpxei.store_callbacks)
    await mpxei.update_q.put({
        'update_type': 'start_ret', 'data': {
            'fd': proj.ctop_fifo.fileno()
        },
    })

@cr.route('stop')
def stop_handler(mpxei, args):
    print('handling stop, args:', args)
    proj = mpxei.pm.proj_fds[args['fd']]
    mpxei.pm.stop(proj)
    await mpxei.update_q.put({
        'update_type': 'stop_ret', 'data': {
            'fd': args['fd']
        }
    })

@cr.route('set_field')
def set_field_handler(mpxei, args):
    print('TODO handling set_field, args:', args)

@cr.route('send_raw')
def send_raw_handler(mpxei, args):
    print('TODO handling send_raw, args:', args)

@cr.route('send_msg')
def send_msg_handler(mpxei, args):
    print('TODO handling send_msg, args:', args)

def cmd_handler(mpxei, cmd):
    cr.handle(mpxei, cmd)
