# -*- coding: utf-8 -*-
import KBEngine
from KBEDebug import *
from interfaces.GameObject import GameObject

class Space(GameObject):
	def __init__(self):
		GameObject.__init__(self)
		DEBUG_MSG('created space[%d] entityID = %i.' % (self.spaceUType, self.id))
		
		KBEngine.addSpaceGeometryMapping(self.spaceID, None, "../../res/Media/Scenes/Scene1")
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
