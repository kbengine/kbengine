# -*- coding: utf-8 -*-
import KBEngine
import d_spaces
from KBEDebug import *
from interfaces.GameObject import GameObject

class Space(GameObject):
	def __init__(self):
		GameObject.__init__(self)
		DEBUG_MSG('created space[%d] res=%s, entityID = %i.' % (self.spaceUType, d_spaces.datas[self.spaceUType].get("resPath", ""), self.id))
		
		KBEngine.addSpaceGeometryMapping(self.spaceID, None, d_spaces.datas[self.spaceUType].get("resPath", ""))
		KBEngine.globalData["space_%i" % self.spaceID] = self.base
	
	def onDestroy(self):
		"""
		KBEngine method.
		"""
		del KBEngine.globalData["space_%i" % self.spaceID]
		
	def onEnter(self, entityMailbox):
		"""
		defined method.
		进入场景
		"""
		pass
		
	def onLeave(self, entityID):
		"""
		defined method.
		离开场景
		"""
		pass
		
Space._timermap = {}
Space._timermap.update(GameObject._timermap)
