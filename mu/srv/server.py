import http.server
import socketserver
from functools import partial

from mu.srv.handler import ReqHandler
from mu.srv.config import get_config
from mu.harness.pm import ProjectManager
from mu.sims.leds import Leds

TCP_PORT = 8989

class ThreadedServer(socketserver.ThreadingMixIn, http.server.HTTPServer):
    def __init__(self, pm, address, handler_class):
        super().__init__(address, handler_class)
        self.pm = pm
        # Temporarily start manually for testing
        self.pm.start('leds', Leds)

    def stop(self):
        self.pm.end()
        self.server_close()


if __name__ == '__main__':
    address = ('', TCP_PORT)
    config = get_config()
    pm = ProjectManager(config=config)
    handler = partial(ReqHandler, pm)
    server = ThreadedServer(pm, address, handler)
    print('now serving musrv on port', TCP_PORT)
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    print('closing musrv')
    server.stop()
