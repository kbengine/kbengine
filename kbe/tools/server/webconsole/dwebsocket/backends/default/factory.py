import logging
import socket
from dwebsocket import factory
from .websocket import DefaultWebSocket
from .protocols import get_websocket_protocol


logger = logging.getLogger(__name__)


class WebSocketFactory(factory.WebSocketFactory): 

	def get_wsgi_sock(self):
		if 'gunicorn.socket' in self.request.META:
			sock = self.request.META['gunicorn.socket'].dup()
		else:
			wsgi_input = self.request.META['wsgi.input']
			if hasattr(wsgi_input, '_sock'):
				sock = wsgi_input._sock
			elif hasattr(wsgi_input, 'rfile'):  # gevent
				sock = wsgi_input.rfile._sock
			elif hasattr(wsgi_input, 'raw'):
				sock = wsgi_input.raw._sock
			else:
				raise ValueError('Socket not found in wsgi.input')
		sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)  
		return sock

	def create_websocket(self):
		if not self.is_websocket():
			return None
		try:
			protocol = get_websocket_protocol(self.get_websocket_version())(
				sock = self.get_wsgi_sock(),
				headers = self.request.META
			)
			return DefaultWebSocket(protocol=protocol)
		except KeyError as e:
			logger.exception(e)
		return None
