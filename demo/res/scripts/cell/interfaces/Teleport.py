# -*- coding: utf-8 -*-
import KBEngine
import SpaceContext
from KBEDebug import * 

class Teleport:
	def __init__(self):
		pass

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
		self.teleportingSpaceUType = spaceUType
		self.teleport(spaceCellMailbox, position, direction)
		
	def onTeleportSuccess(self, nearbyEntity):
		"""
		KBEngine method.
		"""
		self.spaceUType = self.teleportingSpaceUType
		
Teleport._timermap = {}
