# -*- coding: utf-8 -*-
import KBEngine
from KBEDebug import * 
from interfaces.CombatPropertys import CombatPropertys

class Combat(CombatPropertys):
	def __init__(self):
		CombatPropertys.__init__(self)

	def commitFight(self, exposed, targetID, skillID):
		"""
		Exposed method.
		"""
		DEBUG_MSG("Combat::commitFight: targetID=%i, skillID=%i" % (targetID, skillID))
		self.getCurrSpace().commitFight(self.id, targetID, skillID)
		
Combat._timermap = {}
