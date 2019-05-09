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

# SPACE数据类型定义
SPACE_VALUE_TYPE_UNKNOWN		= 0
SPACE_VALUE_TYPE_UINT8			= 1
SPACE_VALUE_TYPE_UINT16			= 2
SPACE_VALUE_TYPE_UINT32			= 3
SPACE_VALUE_TYPE_UINT64			= 4
SPACE_VALUE_TYPE_INT8			= 5
SPACE_VALUE_TYPE_INT16			= 6
SPACE_VALUE_TYPE_INT32			= 7
SPACE_VALUE_TYPE_INT64			= 8
SPACE_VALUE_TYPE_FLOAT			= 9
SPACE_VALUE_TYPE_DOUBLE			= 10
SPACE_VALUE_TYPE_CHAR			= 11
SPACE_VALUE_TYPE_STRING			= 12
SPACE_VALUE_TYPE_BOOL			= 13
SPACE_VALUE_TYPE_COMPONENT_TYPE	= 14



CMD_ID_querySpaceViewer = {
	Define.CELLAPPMGR_TYPE : 50003,
	Define.CELLAPP_TYPE : 50005
}

class SpaceViewer(ServerApp.ServerApp):
	"""
	使用样例：
	import sys
	sys.path.append(r"F:\kbengine\kbe\tools\server")
	import pycommon.SpaceViews
	import pycommon.Define
	w = pycommon.SpaceViews.SpaceViewer(4)
	w.connect("127.0.0.1", 12345) # host, port -> got from Machines.ComponentInfo.{intaddr,intport}
	w.requireQuerySPACE("root")
	w.processOne()
	w.watchData
	"""
	def __init__(self, componentType):
		"""
		"""
		ServerApp.ServerApp.__init__(self)
		self.registerMsg(CONSOLE_PROFILECB_MSGID, self.onSpaceViewerMsg)
		self.SpaceViewerData = []
		self.componentType = componentType
		assert componentType in CMD_ID_querySpaceViewer
		
	def clearSpaceViewerData(self):
		"""
		"""
		self.SpaceViewerData = []
		
	def requireQuerySpaceViewer(self):
		"""
		"""
		CMD_querySpaceViewer = CMD_ID_querySpaceViewer[self.componentType]
		msg = MessageStream.MessageStreamWriter(CMD_querySpaceViewer)
		self.send(msg)

	def onSpaceViewerMsg(self, streamReader):
		"""
		first readUint64()
		size() readUint32()
		id() readUint32()
		getGeomapp0ingPath() readString()
		allCells.size() readString()
		first readUint64()
		"CellAppCID" : [], "SpacesSize" :[], "SpaceID" : [],"SpacePath":[], "ScriptModuleName":[],"SpacePathSize" :[], "CELL_ID" : []
		"""
		#id_ = streamReader.readUint32()
		componentType = streamReader.readInt32()
		componentID = streamReader.readInt64()
		_d = {"ComponentType" : componentType, "ComponentID" : componentID, "cellapp":{}}
		cellapps = _d["cellapp"]

		while not streamReader.EOF():
			CellAppCID = streamReader.readUint64()
			SpacesSize = streamReader.readUint32()
			cellapps["%s" % CellAppCID] = cellapp = { "SpacesSize":SpacesSize, "SpaceID":{}}
			
			for i in range(0, SpacesSize):
				SpaceID = streamReader.readUint32()
				SpacePath = streamReader.readString()
				ScriptModuleName = streamReader.readString()
				SpaceCellSize = streamReader.readUint32()
				cellapp["SpaceID"]["%s" % SpaceID] = space = { "CELL_ID" : [] }
				space["SpacePath"] = SpacePath
				space["ScriptModuleName"] = ScriptModuleName
				for j in range(SpaceCellSize):
					CELL_ID = streamReader.readUint32()
					space["CELL_ID"].append(CELL_ID)

		self.SpaceViewerData = _d




class CellAppMgrViewer(ServerApp.ServerApp):
	"""
	获取CellAppMgr中space信息
	使用样例：
	import sys
	sys.path.append(r"F:\kbengine\kbe\tools\server")
	import pycommon.CellViews
	import pycommon.Define
	w = pycommon.CellViews.CellAppMgrViewer(4,1)
	w.connect("127.0.0.1", 65151)
	w.requireQueryCellAppMgrViewer()
	w.processOne()
	"""
	def __init__(self, componentType, spaceID):
		"""
		"""
		ServerApp.ServerApp.__init__(self)
		self.registerMsg(CONSOLE_PROFILECB_MSGID, self.onCellAppMgrViewerMsg)
		self.CellAppMgrViewerData = []
		self.spaceID = spaceID
		self.componentType = componentType
		
		
	def clearCellAppMgrViewerData(self):
		"""
		"""
		self.CellAppMgrViewerData = []
		
	def requireQueryCellAppMgrViewer(self):
		"""
		"""
		msg = MessageStream.MessageStreamWriter(50004)
		msg.writeBool(0)
		msg.writeUint32(self.spaceID)
		self.send(msg)

	def onCellAppMgrViewerMsg(self, streamReader):
		"""
		"""
		componentType = streamReader.readInt32()
		componentID = streamReader.readInt64()
		_d = {"componentType" : componentType, "componentID" : componentID, "cellappID":[], "spaceID":[], "geomapping":[], "ScriptModuleName":[], "cells_size":[], "cellID":[]}
		while not streamReader.EOF():
			cellappID = streamReader.readUint64()
			_d["cellappID"].append(cellappID)
			spaceID = streamReader.readUint32()
			_d["spaceID"].append(spaceID)
			geomapping = streamReader.readString()
			_d["geomapping"].append(geomapping)
			ScriptModuleName = streamReader.readString()
			_d["ScriptModuleName"].append(ScriptModuleName)
			cells_size = streamReader.readUint32()
			_d["cells_size"].append(cells_size)
			for i in range(0,cells_size):
				cellID = streamReader.readUint32()
				_d["cellID"].append(cellID)
		self.CellAppMgrViewerData = _d

class CellViewer(ServerApp.ServerApp):
	"""
	获取cellapp中entity信息
	使用样例：
	import sys
	sys.path.append(r"F:\kbengine\kbe\tools\server")
	import pycommon.CellViews
	import pycommon.Define
	w = pycommon.CellViews.CellViewer(4,5)
	w.connect("127.0.0.1", 42225)
	w.requireQueryCellViewer()
	w.processOne()
	"""
	def __init__(self, componentType, spaceID):
		"""
		"""
		ServerApp.ServerApp.__init__(self)
		self.registerMsg(CONSOLE_PROFILECB_MSGID, self.onCellViewerMsg)
		self.CellViewerData = []
		self.spaceID = spaceID
		self.componentType = componentType
		self.a = 0
		self.list = {"componentType" : [], "componentID" : [], "type":[], "scriptModules_size":[], "UType":[], "Name":[],"spaceEntity":{}}
		assert componentType in CMD_ID_querySpaceViewer
		
	def clearCellViewerData(self):
		"""
		"""
		self.CellViewerData = []
		self.list = {"componentType" : "", "componentID" :"", "type":"", "scriptModules_size":[], "UType":[], "Name":[],"spaceEntity":{}}

		
	def requireQueryCellViewer(self):
		"""
		"""
		CMD_queryCellViewer = CMD_ID_querySpaceViewer[self.componentType]
		msg = MessageStream.MessageStreamWriter(CMD_queryCellViewer)
		msg.writeBool(0)
		msg.writeUint32(self.spaceID)
		msg.writeUint32(0)
		self.send(msg)

	def onCellViewerMsg(self, streamReader):
		""" 
		"""
		componentType = streamReader.readInt32()
		componentID = streamReader.readInt64()
		CPtype = streamReader.readInt32()
		self.list["componentType"] = componentType
		self.list["componentID"] = componentID
		self.list["type"] = CPtype
		while not streamReader.EOF():
			if self.a == 0:
				scriptModules_size = streamReader.readUint32()
				self.list["scriptModules_size"].append(scriptModules_size)
				if scriptModules_size != "" or scriptModules_size != None :
					for i in range(0,scriptModules_size):
						UType = streamReader.readUint16()
						self.list["UType"].append(UType) 
						if CPtype == 0:
							Name = streamReader.readString()
							self.list["Name"].append(Name)
				self.a = self.a +1	
			else:
				viewerIter = streamReader.readInt32()
				bools = streamReader.readBool()
				self.list["spaceEntity"]["%s" % viewerIter] = {}
				if bools != False or bools != 0:
					self.list["spaceEntity"]["%s" % viewerIter]["update"] = bools
					pEntity = streamReader.readUint16()
					self.list["spaceEntity"]["%s" % viewerIter]["pEntity"] = pEntity
					position_X = streamReader.readFloat()
					self.list["spaceEntity"]["%s" % viewerIter]["position_X"] = ""
					self.list["spaceEntity"]["%s" % viewerIter]["position_X"] = position_X
					position_Y = streamReader.readFloat()
					self.list["spaceEntity"]["%s" % viewerIter]["position_Y"] = ""
					self.list["spaceEntity"]["%s" % viewerIter]["position_Y"] = position_Y
					position_Z = streamReader.readFloat()
					self.list["spaceEntity"]["%s" % viewerIter]["position_Z"] = ""
					self.list["spaceEntity"]["%s" % viewerIter]["position_Z"] = position_Z
					direction_roll = streamReader.readFloat()
					self.list["spaceEntity"]["%s" % viewerIter]["direction_roll"] = ""
					self.list["spaceEntity"]["%s" % viewerIter]["direction_roll"] = direction_roll
					direction_pitch = streamReader.readFloat()
					self.list["spaceEntity"]["%s" % viewerIter]["direction_pitch"] = ""
					self.list["spaceEntity"]["%s" % viewerIter]["direction_pitch"] = direction_pitch
					direction_yaw = streamReader.readFloat()
					self.list["spaceEntity"]["%s" % viewerIter]["direction_yaw"] = ""
					self.list["spaceEntity"]["%s" % viewerIter]["direction_yaw"] = direction_yaw
				else:
					self.list["spaceEntity"]["%s" % viewerIter]["update"] = bools
		self.CellViewerData = self.list

