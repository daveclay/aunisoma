from websocket_server import WebsocketServer
import json
import main


def init(payload, client, server):
    panels_dict = list(map(lambda panel: panel.to_dict(), main.PANEL_CONTEXT.panels))
    message_dict = {
        'type': 'panelList',
        'panels': panels_dict
    }
    message = json.dumps(message_dict)
    server.send_message(client, message)


handlers = {
    'init': init
}


# Called for every client connecting (after handshake)
def new_client(client, server):
    print("New client connected and was given id %d" % client['id'])


# Called for every client disconnecting
def client_left(client, server):
    print("Client(%d) disconnected" % client['id'])


# Called when a client sends a message
def message_received(client, server, message):
    message = json.loads(message)
    command = message['type']
    handlers[command](message['payload'], client, server)
    print("Client(%d) said: %s" % (client['id'], message))


PORT = 9001
server = WebsocketServer(port=PORT)
server.set_fn_new_client(new_client)
server.set_fn_client_left(client_left)
server.set_fn_message_received(message_received)
server.run_forever()
