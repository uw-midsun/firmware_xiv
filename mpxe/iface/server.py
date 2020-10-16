import http.server
import socketserver
import threading
import asyncio
import websockets
import sys

import os
os.chdir('mpxe/iface/static')



async def rx_handler(websocket, path):
    async for message in websocket:
        print('rxed', message)
        await websocket.send('rx success')

async def tx_handler(websocket, path):
    while True:
        await websocket.send('ping')
        await asyncio.sleep(3)

async def ws_handle(websocket, path):
    tx_task = asyncio.ensure_future(tx_handler(websocket, path))
    rx_task = asyncio.ensure_future(rx_handler(websocket, path))
    done, pending = await asyncio.wait(
        [tx_task, rx_task],
        return_when=asyncio.FIRST_COMPLETED,
    )
    for task in pending:
        task.cancel()

def run():
    PORT = 8080
    handler = http.server.SimpleHTTPRequestHandler
    try:
        httpd = socketserver.TCPServer(('', PORT), handler)
    except OSError as o:
        print('http server failed')
        sys.exit(1)
    print('serving at port', PORT)
    try:
        httpd.serve_forever()
    except KeyboardInterrupt as k:
        httpd.shutdown()
        raise k

if __name__ == '__main__':
    t = threading.Thread(target=run)
    t.start()
    server = websockets.serve(ws_handle, "192.168.24.24", 8082)
    asyncio.get_event_loop().run_until_complete(server)
    asyncio.get_event_loop().run_forever()
