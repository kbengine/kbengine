import uwsgi
from dwebsocket.backends.default.factory import WebSocketFactory
from dwebsocket.backends.default.protocols import get_websocket_protocol


class SocketWarp(object):

    def __init__(self, request):
        self.request = request
        self.closed = False


    def fileno(self):
        return self.request.META["wsgi.input"].fileno()

    def recv(self, bufsize):
        if not self.closed:
            uwsgi.wait_fd_read(self.fileno(), -1)
            uwsgi.suspend()
        if not self.closed:
            return uwsgi.recv(self.fileno(), bufsize)

    def send(self, body):
        if not self.closed:
            uwsgi.send(self.fileno(), body)
        return len(body)

    def sendall(self, body):
        self.send(body)

    def close(self):
        self.closed = True
        uwsgi.close(self.fileno())

class uWsgiWebSocketFactory(WebSocketFactory): 

    def get_wsgi_sock(self):
        return SocketWarp(self.request)