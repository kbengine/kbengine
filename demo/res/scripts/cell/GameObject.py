# -*- coding: utf-8 -*-
import KBEngine
from KBEDebug import * 
import d_spaces
import SpaceContext
import wtimer

class GameObject(KBEngine.Entity):
	def __init__(self):
		KBEngine.Entity.__init__(self)

	def getScriptName(self):
		return self.__class__.__name__
		
	def onTimer(self, tid, userArg):
		DEBUG_MSG("%s::onTimer: %i, tid:%i, arg:%i" % (self.getScriptName(), self.id, tid, userArg))
		self._timermap[userArg](self, tid)
		
	def getCurrSpaceBase(self):
		"""
		获得当前space的entity baseMailbox
		"""
		spaceBase = KBEngine.globalData["space_%i" % self.spaceID]
		return spaceBase

	def getCurrSpace(self):
		"""
		获得当前space的entity baseMailbox
		"""
		spaceBase = self.getCurrSpaceBase()
		return KBEngine.entities[spaceBase.id]
		
	def getSpaceMgr(self):
		"""
		获取场景管理器
		"""
		return KBEngine.globalData["SpaceMgr"]
		
	def teleportSpace(self, spaceUType, position, direction, context):
		"""
		defined.
		传送到某场景
		"""
		assert self.base != None
		self.getSpaceMgr().teleportSpace(self.base, spaceUType, position, direction, SpaceContext.createContext(self, spaceUType))
		
	def onTeleportSpaceCB(self, spaceCellMailbox, spaceUType, position, direction):
		"""
		defined.
		baseapp返回teleportSpace的回调
		"""
		DEBUG_MSG("Avatar::onTeleportSpaceCB: %i mb=%s, spaceUType=%i, pos=%s, dir=%s." % \
					(self.id, spaceCellMailbox, spaceUType, position, direction))
					
		self.teleport(spaceCellMailbox, position, direction)

GameObject._timermap = {}
