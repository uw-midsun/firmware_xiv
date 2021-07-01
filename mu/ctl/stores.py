import json
import pprint
import tempfile
import subprocess

import mu.ctl.req as req


def view(args):
    params = {'sim': args.sim, 'type': args.store_type, 'key': args.key}
    r = req.send('view', params)
    if r.status_code == 200:
        pprint.pprint(json.loads(r.text), indent=2)
    else:
        print(r.text)


def update(args):
    params = {'sim': args.sim, 'type': args.store_type, 'key': args.key}
    r = req.send('view', params)
    if r.status_code != 200:
        print(r.text)
        return
    tf = tempfile.NamedTemporaryFile(suffix='.tmp')
    tf.write(r.text.encode())
    tf.flush()

    subprocess.call(['vim', tf.name])

    tf.seek(0)
    edited = tf.read().decode()

    r = req.send('apply', params, edited)
    print(r.text)


def get_io(args):
    params = {'name': args.name}
    r = req.send('get', params)
    print(r.text)


def set_io(args):
    params = {'name': args.name, 'val': args.val}
    r = req.send('set', params)
    print(r.text)
