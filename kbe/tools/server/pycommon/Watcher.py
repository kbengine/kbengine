# -*- coding: utf-8 -*-

import traceback

from . import Define, ServerApp, MessageStream


CONSOLE_COMMANDCB_MSGID	= 65500
CONSOLE_LOG_MSGID		= 65501
CONSOLE_WATCHERCB_MSGID	= 65502
CONSOLE_PROFILECB_MSGID	= 65503



# watcher数据类型定义
WATCHER_VALUE_TYPE_UNKNOWN			= 0
WATCHER_VALUE_TYPE_UINT8			= 1
WATCHER_VALUE_TYPE_UINT16			= 2
WATCHER_VALUE_TYPE_UINT32			= 3
WATCHER_VALUE_TYPE_UINT64			= 4
WATCHER_VALUE_TYPE_INT8				= 5
WATCHER_VALUE_TYPE_INT16			= 6
WATCHER_VALUE_TYPE_INT32			= 7
WATCHER_VALUE_TYPE_INT64			= 8
WATCHER_VALUE_TYPE_FLOAT			= 9
WATCHER_VALUE_TYPE_DOUBLE			= 10
WATCHER_VALUE_TYPE_CHAR				= 11
WATCHER_VALUE_TYPE_STRING			= 12
WATCHER_VALUE_TYPE_BOOL				= 13
WATCHER_VALUE_TYPE_COMPONENT_TYPE	= 14

CMD_ID_queryWatcher = {
	Define.BASEAPP_TYPE    : 41001,
	Define.CELLAPP_TYPE    : 41002,
	Define.LOGINAPP_TYPE   : 41003,
	Define.BASEAPPMGR_TYPE : 41004,
	Define.CELLAPPMGR_TYPE : 41005,
	Define.DBMGR_TYPE      : 41006,
	Define.INTERFACES_TYPE : 41007,
	Define.LOGGER_TYPE     : 41008,
}

class Watcher(ServerApp.ServerApp):
	"""
	使用样例：
	import sys
	sys.path.append(r"x:\kbengine\kbe\tools\server")
	import pycommon.Watcher
	import pycommon.Define
	w = pycommon.Watcher.Watcher(pycommon.Define.CELLAPP_TYPE)
	w.connect("127.0.0.1", 12345) # host, port -> got from Machines.ComponentInfo.{intaddr,intport}
	w.requireQueryWatcher("root")
	w.processOne()
	w.watchData
	"""
	def __init__(self, componentType):
		"""
		"""
		ServerApp.ServerApp.__init__(self)
		self.registerMsg(CONSOLE_WATCHERCB_MSGID, self.onWatcherMsg)
		self.watchData = []
		self.componentType = componentType
		assert componentType in CMD_ID_queryWatcher
		
	def clearWatchData(self):
		"""
		"""
		self.watchData = []
		
	def requireQueryWatcher(self, path):
		"""
		"""
		CMD_queryWatcher = CMD_ID_queryWatcher[self.componentType]
		msg = MessageStream.MessageStreamWriter(CMD_queryWatcher)
		msg.writeString(path)
		self.send(msg)

	def onWatcherMsg(self, streamReader):
		"""
		"""
		type = streamReader.readUint8()
		_d = {"type" : type, "path" : "", "values" : {}, "keys" : []}
		if type == 0:
			while not streamReader.EOF():
				path = streamReader.readString()
				name = streamReader.readString()
				id = streamReader.readUint16()
				wtype = streamReader.readUint8()
				fullpath = "%s%s%s" % (path, len(path) > 0 and "/" or "", name)
				
				if wtype == WATCHER_VALUE_TYPE_UINT8:
					val = streamReader.readUint8()
				elif wtype == WATCHER_VALUE_TYPE_UINT16:
					val = streamReader.readUint16()
				elif wtype == WATCHER_VALUE_TYPE_UINT32:
					val = streamReader.readUint32()
				elif wtype == WATCHER_VALUE_TYPE_UINT64:
					val = streamReader.readUint64()
				elif wtype == WATCHER_VALUE_TYPE_INT8:
					val = streamReader.readInt8()
				elif wtype == WATCHER_VALUE_TYPE_INT16:
					val = streamReader.readInt16()
				elif wtype == WATCHER_VALUE_TYPE_INT32:
					val = streamReader.readInt32()
				elif wtype == WATCHER_VALUE_TYPE_INT64:
					val = streamReader.readInt64()
				elif wtype == WATCHER_VALUE_TYPE_FLOAT:
					val = streamReader.readFloat()
				elif wtype == WATCHER_VALUE_TYPE_DOUBLE:
					val = streamReader.readDouble()
				elif wtype == WATCHER_VALUE_TYPE_CHAR:
					val = streamReader.read(1).decode()
				elif wtype == WATCHER_VALUE_TYPE_STRING:
					val = streamReader.readString()
				elif wtype == WATCHER_VALUE_TYPE_BOOL:
					val = streamReader.readBool()
				elif wtype == WATCHER_VALUE_TYPE_COMPONENT_TYPE:
					val = streamReader.readInt32()
				else:
					assert False, "not support!"
				
				#_w = { "fullpath" : fullpath, "id" : id, "val" : val }
				
				_d["path"] = path
				_d["values"][name] = val
		
		else:
			rootpath = streamReader.readString()
			if rootpath == "/":
				rootpath = ""

			if rootpath:
				_d["path"] = rootpath

			while not streamReader.EOF():
				path = streamReader.readString()
				_d["keys"].append(path)

		#print("Watcher::onWatcherMsg(), %s" % (_d))
		self.watchData.append(_d)
