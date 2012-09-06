# -*- coding: utf-8 -*-
import KBEngine
from KBEDebug import * 

class GameObject(KBEngine.Entity):
	def __init__(self):
		KBEngine.Entity.__init__(self)

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
		
	def teleportSpace(self, spaceUType, position, direction):
		"""
		defined.
		传送到某场景
		"""
		assert self.base != None
		self.getSpaceMgr().teleportSpace(self.base, spaceUType, position, direction)
		
	def onTeleportSpaceCB(self, spaceCellMailbox, spaceUType, position, direction):
		"""
		defined.
		baseapp返回teleportSpace的回调
		"""
		DEBUG_MSG("Avatar::onTeleportSpaceCB: %i mb=%s, spaceUType=%i, pos=%s, dir=%s." % \
					(self.id, spaceCellMailbox, spaceUType, position, direction))
					
		self.teleport(spaceCellMailbox, position, direction)

