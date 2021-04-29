import http.server

class ReqHandler(http.server.BaseHTTPRequestHandler):
    def __init__(self, pm):
        super().__init__()
        self.pm = pm

    def do_GET(self):
        self.send_response(200)
        self.end_headers()
        self.wfile.write('unimplemented!\n'.encode())
