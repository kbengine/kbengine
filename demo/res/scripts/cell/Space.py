# -*- coding: utf-8 -*-
import KBEngine
from KBEDebug import *
from interfaces.GameObject import GameObject

class Space(GameObject):
	def __init__(self):
		GameObject.__init__(self)
		DEBUG_MSG('created space[%d] entityID = %i.' % (self.spaceUType, self.id))
		#self.addSpaceGeometryMapping(self.spaceID, self.spaceName)

		KBEngine.globalData["space_%i" % self.spaceID] = self.base
	
Space._timermap = {}
Space._timermap.update(GameObject._timermap)
