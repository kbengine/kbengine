# -*- coding: utf-8 -*-
import KBEngine
from KBEDebug import * 

class Dialog:
	def __init__(self):
		pass

	def dialog_addOption(self, dialogType, dialogKey, title, extra):
		"""
		define method.
		"""
		DEBUG_MSG("Dialog:dialog_addOption::dialogType=%i, dialogKey=%i, title=%s, extra=%s" % \
				(dialogType, dialogKey, title, extra))

	def dialog_setText(self, body, isPlayer, headID, sayname):
		"""
		define method.
		"""
		DEBUG_MSG("Dialog:dialog_setText::body=%s, isPlayer=%i, headID=%i, sayname=%s" % \
				(body, isPlayer, headID, sayname))
	
	def dialog_close(self):
		"""
		define method.
		"""
		DEBUG_MSG("Dialog:dialog_close:: %i" % (self.id))
				
Dialog._timermap = {}
