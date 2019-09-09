# -*- coding: utf-8 -*-
import socket, select
import sys
import os
import struct
import traceback
import select
import getpass
import time

from . import Define, MessageStream, ServerApp

CONSOLE_PROFILECB_MSGID = 65503

# ComponentStatus数据类型定义
ComponentStatus_VALUE_TYPE_UNKNOWN			= 0
ComponentStatus_VALUE_TYPE_UINT8			= 1
ComponentStatus_VALUE_TYPE_UINT16			= 2
ComponentStatus_VALUE_TYPE_UINT32			= 3
ComponentStatus_VALUE_TYPE_UINT64			= 4
ComponentStatus_VALUE_TYPE_INT8				= 5
ComponentStatus_VALUE_TYPE_INT16			= 6
ComponentStatus_VALUE_TYPE_INT32			= 7
ComponentStatus_VALUE_TYPE_INT64			= 8
ComponentStatus_VALUE_TYPE_FLOAT			= 9
ComponentStatus_VALUE_TYPE_DOUBLE			= 10
ComponentStatus_VALUE_TYPE_CHAR				= 11
ComponentStatus_VALUE_TYPE_STRING			= 12
ComponentStatus_VALUE_TYPE_BOOL				= 13
ComponentStatus_VALUE_TYPE_COMPONENT_TYPE	= 14

CMD_ID_queryAppsLoads = {
	Define.BASEAPPMGR_TYPE : 50001,
	Define.CELLAPPMGR_TYPE : 50002,
}
class ComponentStatus(ServerApp.ServerApp):
	"""docstring for ComponentStatus"""
	def __init__(self, componentType):
		"""
		"""
		ServerApp.ServerApp.__init__(self)
		self.registerMsg(CONSOLE_PROFILECB_MSGID, self.onComponentStatusMsg)
		self.CSData = []
		self.componentType = componentType
		assert componentType in CMD_ID_queryAppsLoads

	def clearCSData(self):
		"""
		"""
		self.watchData = []
		
	def requireQueryCS(self):
		CMD_queryAppsLoads = CMD_ID_queryAppsLoads[self.componentType]
		msg = MessageStream.MessageStreamWriter(CMD_queryAppsLoads)
		self.send(msg)
		
	def onComponentStatusMsg(self, streamReader):
		if self.componentType == 3:
			_d = {"componentID" : [], "load" : [], "numBases" : [], "numEntities" : [], "numProxices":[], "flags":[]}
			while not streamReader.EOF():
				x1 = streamReader.readUint64()
				x2 = streamReader.readFloat()
				x3 = streamReader.readInt32()
				x4 = streamReader.readInt32()
				x5 = streamReader.readInt32()
				x6 = streamReader.readUint32()
				_d["componentID"].append(x1)
				_d["load"].append(x2)
				_d["numBases"].append(x3)
				_d["numEntities"].append(x4)
				_d["numProxices"].append(x5)
				_d["flags"].append(x6)
			self.CSData = _d
		else:
			_d = {"componentID" : [], "load" : [], "numEntities" : [], "flags":[]}
			while not streamReader.EOF():
				x1 = streamReader.readUint64()
				x2 = streamReader.readFloat()
				x3 = streamReader.readInt32()
				x4 = streamReader.readUint32()
				_d["componentID"].append(x1)
				_d["load"].append(x2)
				_d["numEntities"].append(x3)
				_d["flags"].append(x4)
			self.CSData = _d
