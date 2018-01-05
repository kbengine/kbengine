# -*- coding: utf-8 -*-
import KBEngine
from KBEDebug import * 

class Dialog:
	def __init__(self):
		pass

	def dialog_addOption(self, dialogType, dialogKey, title, extra):
		"""
		defined method.
		"""
		DEBUG_MSG("Dialog:dialog_addOption::dialogType=%i, dialogKey=%i, title=%s, extra=%s" % \
				(dialogType, dialogKey, title, extra))

	def dialog_setText(self, body, isPlayer, headID, sayname):
		"""
		defined method.
		"""
		DEBUG_MSG("Dialog:dialog_setText::body=%s, isPlayer=%i, headID=%i, sayname=%s" % \
				(body, isPlayer, headID, sayname))
	
	def dialog_close(self):
		"""
		defined method.
		"""
		DEBUG_MSG("Dialog:dialog_close:: %i" % (self.id))
				

