import requests
from mu.srv.server import TCP_PORT

<<<<<<< HEAD

def send(route, body=None):
=======
def send(route, params=None, body=None):
>>>>>>> master
    url = 'http://localhost:{}/{}'.format(TCP_PORT, route)
    return requests.get(url, params=params, json=body)
