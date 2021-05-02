import json

def get_routes():
    with open('/home/vagrant/shared/firmware_xiv/mu/srv/routes.json') as f:
        routes = json.load(f)
        return routes
