from websocket_server import WebsocketServer
import time
import json
import assembly
import threading
import logging
import scripts

logging.basicConfig(level=logging.WARN)
logger = logging.getLogger(__name__)


def serialize_panels():
    return list(map(lambda panel: panel.to_dict(), assembly.panel_context.panels))


class ClientManager:
    def __init__(self,
                 client,
                 server,
                 panel_context,
                 sensors):
        self.client = client
        self.server = server
        self.panel_context = panel_context
        self.sensors = sensors
        self.initialized = False
        self.handlers = {
            'init': lambda payload: self.init(payload),
            'panelActive': lambda payload: self.on_panel_active(payload),
            'panelInactive': lambda payload: self.on_panel_inactive(payload),
        }

    def send_message(self, type, payload):
        message_dict = {
            'type': type,
            'payload': payload
        }
        message = json.dumps(message_dict)
        server.send_message(self.client, message)

    def init(self, payload):
        self.send_message('init', {
            'panels': serialize_panels(),
            'interactionConfig': self.panel_context.interaction_config.to_dict()
        })
        self.initialized = True

    def send_update_panels(self):
        if self.initialized:
            self.send_message('updatePanels', {
                'panels': serialize_panels()
            })

    def on_panel_active(self, payload):
        panel_index = payload['panelIndex']
        self.sensors[panel_index] = True

    def on_panel_inactive(self, payload):
        panel_index = payload['panelIndex']
        self.sensors[panel_index] = False

    def handle_message(self, message):
        message_type = message['type']
        handler = self.handlers[message_type]
        handler(message['payload'])


clients = {}


# Called for every client connecting (after handshake)
def new_client(client, server):
    print("New client connected and was given id %d" % client['id'])
    clients[client['id']] = ClientManager(client,
                                          server,
                                          assembly.panel_context,
                                          assembly.sensors)


# Called for every client disconnecting
def client_left(client, server):
    print("Client(%d) disconnected" % client['id'])
    del clients[client['id']]


# Called when a client sends a message
def message_received(client, server, message):
    try:
        message = json.loads(message)
        client_manager = clients[client['id']]
        client_manager.handle_message(message)
        print("Client(%d) said: %s" % (client['id'], message))
    except Exception as error:
        print("Caught error from client %d:" % client['id'])
        logger.exception(error)

def loop():
    # scripts.check_scripts()
    # start = time.monotonic_ns()
    assembly.panel_context.event_loop()
    # print((time.monotonic_ns() - start) / 1000000)

    for client in clients.values():
        client.send_update_panels()

    threading.Timer(.01, loop).start()


loop()

PORT = 9001
server = WebsocketServer(port=PORT)
server.set_fn_new_client(new_client)
server.set_fn_client_left(client_left)
server.set_fn_message_received(message_received)
server.run_forever()
