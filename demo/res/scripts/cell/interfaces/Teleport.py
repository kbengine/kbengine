# -*- coding: utf-8 -*-
import KBEngine
import SpaceContext
from KBEDebug import * 

class Teleport:
	def __init__(self):
		pass

	def onDestroy(self):
		"""
		entity销毁
		"""
		self.getCurrSpaceBase().logoutSpace(self.id)
		
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
		DEBUG_MSG("Teleport::onTeleportSpaceCB: %i mb=%s, spaceUType=%i, pos=%s, dir=%s." % \
					(self.id, spaceCellMailbox, spaceUType, position, direction))
		
		
		self.getCurrSpaceBase().onLeave(self.id)
		self.teleport(spaceCellMailbox, position, direction)
		
	def onTeleportSuccess(self, nearbyEntity):
		"""
		KBEngine method.
		"""
		DEBUG_MSG("Teleport::onTeleportSuccess: %s" % (nearbyEntity))
		self.getCurrSpaceBase().onEnter(self.base)
		self.spaceUType = self.getCurrSpace().spaceUType
		
Teleport._timermap = {}
