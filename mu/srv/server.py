import http.server
import socketserver
import threading

from mu.srv.handler import ReqHandler
from mu.harness.pm import ProjectManager

TCP_PORT = 8989

class ThreadedServer(socketserver.ThreadingMixIn, http.server.HTTPServer):
    def __init__(self, address, handler_class):
        super().__init__(address, handler_class)
        self.pm = ProjectManager()
    
    def server_close(self):
        self.pm.stop_all()
        super().server_close()


def entrypoint():
    address = ('', TCP_PORT)
    httpd = ThreadedServer(address, ReqHandler)
    try:
        print('now serving musrv on port', TCP_PORT)
        httpd.serve_forever()
    except KeyboardInterrupt:
        pass
    httpd.server_close()
