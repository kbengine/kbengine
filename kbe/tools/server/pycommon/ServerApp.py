# -*- coding: utf-8 -*-

import socket
import sys, os, struct
import traceback
import select

from . import Define
from . import MessageStream


class ServerApp:
	def __init__(self):
		"""
		"""
		self.socket_ = None
		self.buffer_ = "".encode() # py25兼容
		self.msgProcesser_ = {} # { msgID : func, ... }
		
	def connect(self, host, port):
		"""
		"""
		assert self.socket_ is None
		self.socket_ = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		self.socket_.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
		self.socket_.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 5 * 1024 * 1024)
		self.socket_.setsockopt(socket.SOL_SOCKET, socket.SO_KEEPALIVE, 1)
		#self.socket_.setblocking(False)
		self.socket_.connect((host, port))
	
	def close(self):
		"""
		"""
		if self.socket_:
			self.socket_.close()
			self.socket_ = None
	
	def connected(self):
		"""
		"""
		return self.socket_ is not None
		
	def registerMsg(self, msgID, func):
		"""
		"""
		assert msgID not in self.msgProcesser_
		self.msgProcesser_[msgID] = func
		
	def deregisterMsg(self, msgID):
		"""
		"""
		self.msgProcesser_.pop(msgID)

	def send(self, streamWriter):
		"""
		"""
		msg = streamWriter.build()
		r = self.socket_.send( msg )
		assert r == len(msg), "r = %s, len(msg) = %s" % (r, len(msg))
		
	def recv(self):
		"""
		"""
		return self.socket_.recv(4096)

	def processOne(self, timeout = 0.1):
		"""
		"""
		if not self.connected():
			return
			
		rl, wl, el = select.select([self.socket_.fileno()], [], [], timeout)
		if rl:
			data = self.socket_.recv(4096)
			if len(data) == 0:
				self.socket_.close()
				self.socket_ = None
			
			self.buffer_ += data
			return self.processMsg()
	
	def processMsg(self):
		"""
		"""
		while len(self.buffer_) > 0:
			msgID = struct.unpack("=H", self.buffer_[:2])[0]
			assert msgID in self.msgProcesser_, "unknown message id %s" % msgID
			
			msgLength = struct.unpack("=H", self.buffer_[2:4])[0] + 4
			if len(self.buffer_) < msgLength:
				return
			
			stream = MessageStream.MessageStreamReader(self.buffer_[4:msgLength])
			self.buffer_ = self.buffer_[msgLength:]
			self.msgProcesser_[msgID](stream)
