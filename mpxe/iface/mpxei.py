import http.server
import socketserver
import threading
import asyncio
import websockets
import sys
import queue
import json

import os
os.chdir('mpxe/iface/static')

from mpxe.harness.pm import ProjectManager
from mpxe.harness.project import Project
from mpxe.harness.canio import Msg

PAGE_PORT = 8080
WS_PORT = 8081
BOX_IP = '192.168.24.24' # this is hardcoded in the Vagrantfile

class Mpxei:
    def __init__(self, bus_name='vcan0'):
        # queue for txing stuff to js
        # format should be {'update_type': 'type', 'data': {data}}
        self.update_q = queue.Queue

        self.pm = ProjectManager(can_bus_name=bus_name)

        # setup for running the page server
        self.page_handler = http.serveer.SimpleHTTPRequestHandler
        self.page_server = socketserver.TCPServer(('', PAGE_PORT), self.page_handler)
        self.page_thread = threading.Thread(target=self.serve_page)
        self.page_thread.start()
        
        # setup for responding to web sockets
        self.ws_server = websockets.serve(self.ws_handle, BOX_IP, WS_PORT)
        self.pm.log_callbacks.append(asyncio.get_event_loop()
            .call_soon_threadsafe(self.log_callback))
        self.pm.store_callbacks.append(asyncio.get_event_loop()
            .call_soon_threadsafe(self.store_callback))
        self.pm.can_callbacks.append(asyncio.get_event_loop()
            .call_soon_threadsafe(self.can_callback))
        self.ws_task_funcs = [self.tx_handler, self.rx_handler]
        asyncio.get_event_loop().run_until_complete(ws_server)

    # wrapper functions for structuring data to send to js
    async def log_callback(self, proj, pm, log):
        self.update_q.put({
            'update_type': 'log', 'data': {
                'proj': proj.ctop_fifo.fileno(),
                'log': log
            },
        })

    async def store_callback(self, proj, pm, store_info):
        self.update_q.put({
            'update_type': 'store', 'data': {
                'proj': proj.ctop_fifo.fileno(),
                'store_type': store_info.type,
                'store_data': 'TODO',
            },
        })
    
    async def can_callback(self, msg):
        self.update_q.put({
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
            # TODO: call command handlers
            print('rxed', message)
            await websocket.send('ret')

    async def ws_handle(self, websocket, path):
        tasks = []
        for task_func in self.ws_task_funcs:
            tasks.append(asyncio.ensure_future(task_func(websocket, path)))
        done, pending = await asyncio.wait(tasks, return_when=asyncio.FIRST_COMPLETED)
        for task in pending:
            task.cancel()

    def serve_page(self):
        self.page_server.serve_forever()

    def end(self):
        self.page_server.shutdown()

    def run(self):
        asyncio.get_event_loop().run_forever()

if __name__ == '__main__':
    mpxei = Mpxei() if len(sys.argv) > 1 else Mpxei(bus_name=sys.argv[1])
    try:
        mpxei.run()
    except KeyboardInterrupt as k:
        mpxei.end()
        raise k
