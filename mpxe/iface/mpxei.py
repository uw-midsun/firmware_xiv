import http.server
import socketserver
import threading
import asyncio
import websockets
import sys
import queue
import json
import os

from mpxe.harness.pm import ProjectManager
from mpxe.harness.project import Project
from mpxe.harness.canio import Msg
from mpxe.iface import cmd

PAGE_PORT = 8080
WS_PORT = 8081
BOX_IP = '192.168.24.24' # this is hardcoded in the Vagrantfile

class Mpxei:
    def __init__(self, bus_name='vcan0'):
        # queue for txing stuff to js
        # format should be {'update_type': 'type', 'data': {data}}
        self.update_q = asyncio.Queue()
        self.loop = asyncio.get_event_loop()

        self.pm = ProjectManager(can_bus_name=bus_name)

        # setup for running the page server
        # self.page_handler = http.server.SimpleHTTPRequestHandler
        # self.page_server = socketserver.TCPServer(('', PAGE_PORT), self.page_handler)
        # self.page_thread = threading.Thread(target=self.serve_page)
        # self.page_thread.start()

        # setup for responding to web sockets
        self.ws_server = websockets.serve(self.ws_handle, BOX_IP, WS_PORT)
        self.pm.can.callbacks.append(self.can_callback)
        self.ws_task_funcs = [self.tx_handler, self.rx_handler]
        asyncio.get_event_loop().run_until_complete(self.ws_server)

    # wrapper functions for structuring data to send to js
    def log_callback(self, proj, pm, log):
        self.loop.call_soon_threadsafe(self.update_q.put_nowait, {
            'update_type': 'log', 'data': {
                'fd': proj.ctop_fifo.fileno(),
                'name': proj.name,
                'log': log,
            },
        })

    def store_callback(self, proj, pm, store_info):
        self.loop.call_soon_threadsafe(self.update_q.put_nowait, {
            'update_type': 'displays', 'data': {
                'fd': proj.ctop_fifo.fileno(),
                'displays': proj.sim.displays
            },
        })

    def can_callback(self, msg):
        self.loop.call_soon_threadsafe(self.update_q.put_nowait, {
            'update_type': 'can', 'data': {
                'name': msg.name, 
                'id': msg.arb_id, 
                'data': msg.data
            },
        })

    async def tx_handler(self, websocket, path):
        while True:
            update = await self.update_q.get()
            await websocket.send(json.dumps(update))

    async def rx_handler(self, websocket, path):
        async for message in websocket:
            cmd.cmd_handler(self, json.loads(message))

    async def ws_handle(self, websocket, path):
        tasks = []
        for task_func in self.ws_task_funcs:
            tasks.append(asyncio.ensure_future(task_func(websocket, path)))
        done, pending = await asyncio.wait(tasks, return_when=asyncio.FIRST_COMPLETED)
        for task in pending:
            task.cancel()

    def serve_page(self):
        print('serving page')
        try:
            self.page_server.serve_forever()
        except KeyboardInterrupt as k:
            sys.exit(0)

    def end(self):
        self.page_server.shutdown()

    def run(self):
        print('running event loop')
        try:
            asyncio.get_event_loop().run_forever()
        except KeyboardInterrupt as k:
            return

if __name__ == '__main__':
    os.chdir('mpxe/iface/static') # for serving the page
    mpxei = Mpxei() if len(sys.argv) == 1 else Mpxei(bus_name=sys.argv[1])
    mpxei.run()
    mpxei.end()
    sys.exit(0)
