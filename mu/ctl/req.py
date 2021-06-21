import requests
from mu.srv.server import TCP_PORT

def send(route, params=None, body=None):
    url = 'http://localhost:{}/{}'.format(TCP_PORT, route)
    return requests.get(url, params=params, json=body)
