#encoding:utf-8

class WebSocket(object):
	"""
	A websocket object that handles the details of
	serialization/deserialization to the socket.

	The primary way to interact with a :class:`WebSocket` object is to
	call :meth:`send` and :meth:`wait` in order to pass messages back
	and forth with the browser.
	"""

	def accept_connection(self):
		raise NotImplementedError

	def send(self, message):
		'''
		Send a message to the client. *message* should be convertable to a
		string; unicode objects should be encodable as utf-8.
		'''
		raise NotImplementedError

	def count_messages(self):
		'''
		Returns the number of queued messages.
		'''
		raise NotImplementedError

	def has_messages(self):
		'''
		Returns ``True`` if new messages from the socket are available, else
		``False``.
		'''
		raise NotImplementedError

	def read(self, fallback=None):
		'''
		Return new message or ``fallback`` if no message is available.
		'''
		raise NotImplementedError

	def wait(self):
		'''
		Waits for and deserializes messages. Returns a single message; the
		oldest not yet processed.
		'''
		raise NotImplementedError

	def __iter__(self):
		'''
		Use ``WebSocket`` as iterator. Iteration only stops when the websocket
		gets closed by the client.
		'''
		while True:
			message = self.wait()
			yield message
			if message is None:
				break

	def close(self, code=None, reason=None):
		'''
		Forcibly close the websocket.
		'''
		raise NotImplementedError
