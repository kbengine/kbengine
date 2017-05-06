# -*- coding: utf-8 -*-

import sys, os, struct
import traceback



from . import Define

# 兼容py2.5
try:
	bytes
	try:
		unicode
	except:
		unicode = str
	NULL_TERMINATOR = "\0".encode()
	NULL_BYTES = "".encode()
except:
	bytes = str
	NULL_TERMINATOR = "\0"
	NULL_BYTES = ""


class MessageStreamReader(object):
	def __init__(self, bytesData):
		"""
		"""
		self.stream = bytesData
		self.rpos = 0

	def read(self, nums):
		"""
		"""
		if self.EOF():
			return bytes()
		dat = self.stream[self.rpos:self.rpos + nums]
		self.rpos += nums
		return dat

	def EOF(self):
		"""
		"""
		return self.rpos >= len(self.stream)

	def length(self):
		"""
		"""
		r = len(self.stream) - self.rpos
		return r > 0 and r or 0

	def readBool(self):
		"""
		"""
		return self.readInt8() > 0

	def readInt8(self):
		"""
		"""
		return struct.unpack("=b", self.read(1))[0]

	def readUint8(self):
		"""
		"""
		return struct.unpack("=B", self.read(1))[0]

	def readInt16(self):
		"""
		"""
		return struct.unpack("=h", self.read(2))[0]

	def readUint16(self):
		"""
		"""
		return struct.unpack("=H", self.read(2))[0]

	def readInt32(self):
		"""
		"""
		return struct.unpack("=i", self.read(4))[0]

	def readUint32(self):
		"""
		"""
		return struct.unpack("=I", self.read(4))[0]

	def readInt64(self):
		"""
		"""
		return struct.unpack("=q", self.read(8))[0]

	def readUint64(self):
		"""
		"""
		return struct.unpack("=Q", self.read(8))[0]

	def readFloat(self):
		"""
		"""
		return struct.unpack("=f", self.read(4))[0]

	def readDouble(self):
		"""
		"""
		return struct.unpack("=d", self.read(8))[0]

	def readBlob(self):
		"""
		"""
		len = self.readInt32()
		b = struct.unpack("=%ss" % len, self.read(len))[0]
		return b

	def readString(self):
		"""
		"""
		b = []
		while not self.EOF():
			c = self.read(1)
			if c == NULL_TERMINATOR:
				break
			b.append(c)
		s = NULL_BYTES.join(b) # 为了兼容py2.5，所以不使用b""
		return s.decode("utf-8")







class MessageStreamWriter(object):
	def __init__(self, messageID):
		"""
		"""
		self.msgID = messageID
		self.stream = Define.BytesIO()

	def build(self):
		"""
		"""
		s1 = self.stream.getvalue()
		s2 = struct.pack("=HH", self.msgID, len(s1))
		return s2 + s1

	def write(self, val):
		"""
		"""
		self.stream.write(val)

	def writeBool(self, val):
		"""
		"""
		self.writeInt8(val and 1 or 0)

	def writeInt8(self, val):
		"""
		"""
		self.stream.write( struct.pack("=b", val) )

	def writeUint8(self, val):
		"""
		"""
		self.stream.write( struct.pack("=B", val) )

	def writeInt16(self, val):
		"""
		"""
		self.stream.write( struct.pack("=h", val) )

	def writeUint16(self, val):
		"""
		"""
		self.stream.write( struct.pack("=H", val) )

	def writeInt32(self, val):
		"""
		"""
		self.stream.write( struct.pack("=i", val) )

	def writeUint32(self, val):
		"""
		"""
		self.stream.write( struct.pack("=I", val) )

	def writeInt64(self, val):
		"""
		"""
		self.stream.write( struct.pack("=q", val) )

	def writeUint64(self, val):
		"""
		"""
		self.stream.write( struct.pack("=Q", val) )

	def writeFloat(self, val):
		"""
		"""
		self.stream.write( struct.pack("=f", val) )

	def writeDouble(self, val):
		"""
		"""
		self.stream.write( struct.pack("=d", val) )

	def writeBlob(self, val):
		"""
		"""
		assert isinstance(val, bytes), "value type not match! current val type = %s" % type(val)
		self.stream.write( struct.pack("=I%ss" % len(val), len(val), val) )

	def writeString(self, val):
		"""
		"""
		assert isinstance(val, (bytes, str, unicode)), "value type not match! current val type = %s" % type(val)
		if isinstance(val, bytes):
			self.stream.write( struct.pack("=%ss" % (len(val) + 1), val) ) # 加1是为了有空终结符
		else:
			val = val.encode("utf-8")
			self.stream.write( struct.pack("=%ss" % (len(val) + 1), val) ) # 加1是为了有空终结符
