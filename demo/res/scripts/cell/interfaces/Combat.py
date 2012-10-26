# -*- coding: utf-8 -*-
import KBEngine
from KBEDebug import * 

class Combat:
	def __init__(self):
		pass

	def commitFight(self, exposed, targetID, skillID):
		"""
		Exposed method.
		"""
		DEBUG_MSG("Combat::commitFight: targetID=%i, skillID=%i" % (targetID, skillID))

Combat._timermap = {}
