import http.server
import json
import urllib

from mu.harness import logger

class InternalError(Exception):
    pass

class ReqHandler(http.server.BaseHTTPRequestHandler):
    def __init__(self, pm, *args, **kwargs):
        self.pm = pm
        self.setup_routing()
        super().__init__(*args, **kwargs)

    def setup_routing(self):
        self.routes = {
            'reset': self.reset_pm,
            'start': self.start,
            'stop': self.stop,
            'sims': self.sims,
            'list': self.sim_list,
            'logs': self.logs,
        }

    def respond(self, code, body='{"acknowledged": "true"}'):
        self.send_response(code)
        self.end_headers()
        self.wfile.write(body.encode())

    def do_GET(self):
        try:
            parsed = urllib.parse.urlparse(self.path)
            params = urllib.parse.parse_qs(parsed.query, keep_blank_values=True)
            self.routes[parsed.path[1:]](params=params)
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

    def start(self, params=None):
        try:
            self.pm.start(params['sim'][0], proj_name=params['proj'][0])
            self.respond(200)
        except ValueError as e:
            self.respond(500, body='{}'.format(e))

    def stop(self, params=None):
        try:
            self.pm.stop_name(params['sim'][0])
            self.respond(200)
        except ValueError as e:
            self.respond(500, body='{}'.format(e))

    def sims(self, params=None):
        sim_list = list(self.pm.sim_cat().keys())
        self.respond(200, body=json.dumps(sim_list))

    def sim_list(self, params=None):
        sim_list = self.pm.sim_list()
        self.respond(200, body=json.dumps(sim_list))

    def logs(self, params=None):
        self.respond(200, body='')
        sim_name = params['sim'][0]
        sub = logger.Subscriber(sim_name, tags=[sim_name])
        self.pm.logger.subscribe(sub)
        try:
            while not self.pm.killed:
                try:
                    log = sub.get()
                    self.wfile.write('[{}] {}\n'.format(log.tag, log.msg).encode())
                except logger.NoLog:
                    pass
        except BrokenPipeError as e:
            pass
        finally:
            self.pm.logger.unsubscribe(sub)
