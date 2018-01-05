# -*- coding: utf-8 -*-
import KBEngine
from KBEDebug import * 
import dialogmgr

class Dialog:
	"""
	与NPC对话模块，客户端通过调用dialog来驱动对话协议
	"""
	def __init__(self):
		pass

	#--------------------------------------------------------------------------------------------
	#                              defined
	#--------------------------------------------------------------------------------------------
	def dialog(self, srcEntityID, targetID, dialogID):
		"""
		exposed.
		对一个目标entity施放一个技能
		"""
		if srcEntityID != self.id:
			return
			
		if not KBEngine.entities.has_key(targetID):
			DEBUG_MSG("Dialog::dialog: %i not found targetID:%i" % (self.id, dialogID))
			return
			
		dialogmgr.onGossip(dialogID, self, KBEngine.entities[targetID])


