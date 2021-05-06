import http.server
import json

from mu.srv.router import get_routes
from mu.harness import logger

class InternalError(Exception):
    pass

class ReqHandler(http.server.BaseHTTPRequestHandler):
    def __init__(self, pm, *args, **kwargs):
        self.pm = pm
        self.setup_routing()
        super().__init__(*args, **kwargs)

    def setup_routing(self):
        routes = get_routes()
        self.routes = {
            routes['reset']: self.reset_pm,
            routes['sim_start']: self.sim_start,
            routes['sim_stop']: self.sim_stop,
            routes['sim_cat']: self.sim_cat,
            routes['sim_list']: self.sim_list,
            routes['logs']: self.sim_logs,
        }

    def respond(self, code, body='{"acknowledged": "true"}'):
        self.send_response(code)
        self.end_headers()
        self.wfile.write(body.encode())

    def do_GET(self):
        try:
            path, _, query = self.path.partition('?')
            params=self.parse_query(query)
            self.routes[path[1:]](params=params)
        except InternalError:
            self.respond(500, body='internal error')

    def parse_query(self, query):
        if query == '':
            return []
        param_list = query.split('&')
        ret = {}
        for param in param_list:
            key, _, val = param.partition('=')
            ret[key] = val
        return ret

    def reset_pm(self, params=None):
        self.pm.reset()
        self.respond(200)

    def sim_start(self, params=None):
        self.pm.start(params['proj'], params['sim'])
        self.respond(200)

    def sim_stop(self, params=None):
        self.pm.stop_name(params['sim'])
        self.respond(200)

    def sim_cat(self, params=None):
        sim_list = list(self.pm.sim_cat().keys())
        self.respond(200, body=json.dumps(sim_list))

    def sim_list(self, params=None):
        sim_list = self.pm.sim_list()
        self.respond(200, body=json.dumps(sim_list))

    def sim_logs(self, params=None):
        self.respond(200, body='')
        sub = logger.Subscriber(params['sim'])
        self.pm.logger.subscribe(sub)
        try:
            while True:
                try:
                    log = sub.get()
                    self.wfile.write('[{}] {}\n'.format(log.tag, log.msg).encode())
                except logger.NoLog:
                    pass
        except BrokenPipeError:
            pass
        self.pm.logger.unsubscribe(sub)
