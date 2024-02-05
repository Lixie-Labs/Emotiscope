import asyncio
import websockets

async def handler(websocket, path):
    while True:
        try:
            message = await websocket.recv()
            response = f"Server received: {message}"
            await websocket.send(response)
        except websockets.ConnectionClosedOK:
            break

start_server = websockets.serve(handler, "localhost", 8080)

asyncio.get_event_loop().run_until_complete(start_server)
asyncio.get_event_loop().run_forever()
