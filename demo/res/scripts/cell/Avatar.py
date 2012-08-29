# -*- coding: utf-8 -*-
import KBEngine
from KBEDebug import *
from GameObject import GameObject

class Avatar(GameObject):
	def __init__(self):
		GameObject.__init__(self) 
		#self.addTimer(1, 1, 1)
		
	def onTimer(self, tid, userArg):
		DEBUG_MSG("Avatar::onTimer: %i, tid:%i, arg:%i" % (self.id, tid, userArg))
		
	def spellTarget(self, srcEntityID, skillID, targetID):
		"""
		exposed.
		对一个目标entity施放一个技能
		"""
		if srcEntityID != self.id:
			return
			
		DEBUG_MSG("Avatar::spellTarget(%i):skillID=%i, targetID=%i", self.id, skillID, targetID)