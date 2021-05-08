import requests
from mu.srv.server import TCP_PORT

def send(route, body=None):
    url = 'http://localhost:{}/{}'.format(TCP_PORT, route)
    r = requests.get(url, params=body)
    return r.text
