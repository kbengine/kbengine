# -*- coding: utf-8 -*-
import KBEngine
from KBEDebug import *
from interfaces.GameObject import GameObject

import d_spaces

class Space(KBEngine.Entity, GameObject):
	def __init__(self):
		KBEngine.Entity.__init__(self)
		GameObject.__init__(self)
		
		resPath = d_spaces.datas.get(self.spaceUType)['resPath']
		
		if resPath == "":
			resPath = "../../res/Media/Scenes/Scene1"
			KBEngine.addSpaceGeometryMapping(self.spaceID, None, resPath)
		else:
			KBEngine.addSpaceGeometryMapping(self.spaceID, None, resPath)
		
		DEBUG_MSG('created space[%d] entityID = %i, res = %s.' % (self.spaceUType, self.id, resPath))
		
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
		DEBUG_MSG('Space::onEnter space[%d] entityID = %i.' % (self.spaceUType, entityMailbox.id))
		
	def onLeave(self, entityID):
		"""
		defined method.
		离开场景
		"""
		DEBUG_MSG('Space::onLeave space[%d] entityID = %i.' % (self.spaceUType, entityID))
		
Space._timermap = {}
Space._timermap.update(GameObject._timermap)
