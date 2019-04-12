#encoding:utf-8
import collections
from dwebsocket.websocket import WebSocket

class DefaultWebSocket(WebSocket):
	"""
	A websocket object that handles the details of
	serialization/deserialization to the socket.

	The primary way to interact with a :class:`WebSocket` object is to
	call :meth:`send` and :meth:`wait` in order to pass messages back
	and forth with the browser.
	"""

	def __init__(self, protocol):
		'''
		Arguments:

		- ``socket``: An open socket that should be used for WebSocket
		  communciation.
		- ``protocol``: not used yet.
		- ``version``: The WebSocket spec version to follow (default is 76)
		- ``handshake_reply``: Handshake message that should be sent to the
		  client when ``send_handshake()`` is called.
		- ``handshake_sent``: Whether the handshake is already sent or not.
		  Set to ``False`` to prevent ``send_handshake()`` to do anything.
		'''
		self.protocol = protocol
		self.closed = False
		self._message_queue = collections.deque()

	def accept_connection(self):
		self.protocol.accept_connection()

	def send(self, message):
		'''
		Send a message to the client. *message* should be convertable to a
		string; unicode objects should be encodable as utf-8.
		'''
		if not self.closed:
			self.protocol.write(message)

	def _get_new_messages(self):
		# read as long from socket as we need to get a new message.
		while self.protocol.can_read():
			self._message_queue.append(self.protocol.read())
			if self._message_queue:
				return

	def count_messages(self):
		'''
		Returns the number of queued messages.
		'''
		self._get_new_messages()
		return len(self._message_queue)

	def has_messages(self):
		'''
		Returns ``True`` if new messages from the socket are available, else
		``False``.
		'''
		if self._message_queue:
			return True
		self._get_new_messages()
		if self._message_queue:
			return True
		return False

	def read(self, fallback=None):
		'''
		Return new message or ``fallback`` if no message is available.
		'''
		if self.has_messages():
			return self._message_queue.popleft()
		return fallback

	def wait(self):
		'''
		Waits for and deserializes messages. Returns a single message; the
		oldest not yet processed.
		'''
		while not self._message_queue:
			# Websocket might be closed already.
			if self.closed:
				return None
			# no parsed messages, must mean buf needs more data
			new_data = self.protocol.read()
			if not new_data:
				return None
			self._message_queue.append(new_data)
		return self._message_queue.popleft()

	def close(self, code=None, reason=None):
		'''
		Forcibly close the websocket.
		'''
		if not self.closed:
			self.protocol.close(code, reason)
			self.closed = True
