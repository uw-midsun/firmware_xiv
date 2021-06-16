import http.server
import json
import urllib

from google.protobuf import json_format

from mu.harness import logger, decoder
from mu.harness.project import StoreUpdate
from mu.protogen.stores_pb2 import MuStoreType


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

            'view': self.view,
            'apply': self.apply,
            'get': self.get,
            'set': self.set,
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
            self.respond(500, body=str(e))

    def stop(self, params=None):
        try:
            self.pm.stop_name(params['sim'][0])
            self.respond(200)
        except ValueError as e:
            self.respond(500, body=str(e))

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
        except BrokenPipeError:
            pass
        finally:
            self.pm.logger.unsubscribe(sub)

    def view(self, params=None):
        sim_name = params['sim'][0]
        store_type = params['type'][0]
        store_key = params['key'][0]
        try:
            sim = self.pm.sim_from_name(sim_name)
            if store_type:
                store = sim.stores_str_lookup(store_type, store_key)
                store_json = json_format.MessageToJson(store)
                self.respond(200, body=store_json)
            else:
                store_dicts = {}
                for key, val in sim.stores.items():
                    key_string = '{}/{}'.format(MuStoreType.Name(key[0]), key[1])
                    store_dicts[key_string] = json_format.MessageToDict(val)
                stores_json = json.dumps(store_dicts)
                self.respond(200, body=stores_json)
        except ValueError as e:
            self.respond(500, body=str(e))

    def apply(self, params=None):
        sim_name = params['sim'][0]
        store_type = params['type'][0]
        key = int(params['key'][0], 0) if params['key'][0] else 0

        raw_len = self.headers['Content-Length']
        store_json = self.rfile.read(int(raw_len)).decode()

        store = decoder.store_from_name(store_type)
        try:
            store_dict = json.loads(store_json)
            json_format.Parse(store_dict, store)
        except json_format.ParseError as e:
            self.respond(500, body=str(e))
            return

        mask = decoder.full_mask(store)
        store_enum = MuStoreType.Value(store_type.upper())
        update = StoreUpdate(store, mask, (store_enum, key))

        try:
            sim = self.pm.sim_from_name(sim_name)
            sim.proj.write_store(update)
            self.respond(200)
        except ValueError as e:
            self.respond(500, body=str(e))

    def get(self, params=None):
        io_name = params['name'][0]
        try:
            if io_name:
                val = self.pm.get_io(io_name)
                self.respond(200, body='{}'.format(val))
            else:
                vals = self.pm.get_all_io()
                self.respond(200, body=json.dumps(vals))
        except (KeyError, ValueError, NotImplementedError) as e:
            self.respond(500, body=str(e))

    def set(self, params=None):
        io_name = params['name'][0]
        val = params['val'][0]
        try:
            val = self.pm.set_io(io_name, val)
            self.respond(200)
        except (KeyError, ValueError, NotImplementedError) as e:
            self.respond(500, body=str(e))
