import http.server

class ReqHandler(http.server.BaseHTTPRequestHandler):
    def __init__(self, pm, *args, **kwargs):
        self.pm = pm
        super().__init__(*args, **kwargs)

    def do_GET(self):
        self.send_response(200)
        self.end_headers()
        self.wfile.write('unimplemented!\n'.encode())
