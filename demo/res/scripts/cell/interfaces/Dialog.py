# -*- coding: utf-8 -*-
import KBEngine
from KBEDebug import * 
import dialogmgr

class Dialog:
	def __init__(self):
		pass

	def dialog(self, srcEntityID, targetID, dialogID):
		"""
		exposed.
		对一个目标entity施放一个技能
		"""
		if srcEntityID != self.id:
			return
			
		if not KBEngine.entities.has_key(targetID):
			DEBUG_MSG("Avatar::dialog: %i not found targetID:%i" % (self.id, dialogID))
			return
			
		dialogmgr.onGossip(dialogID, self, KBEngine.entities[targetID])

Dialog._timermap = {}
