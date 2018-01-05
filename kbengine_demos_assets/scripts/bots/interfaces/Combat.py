# -*- coding: utf-8 -*-
import KBEngine
from KBEDebug import * 

class Combat:
	def __init__(self):
		pass

	def recvDamage(self, attackerID, skillID, damageType, damage):
		"""
		defined.
		"""
		DEBUG_MSG("%s::recvDamage: %i attackerID=%i, skillID=%i, damageType=%i, damage=%i" % \
			(self.getScriptName(), self.id, attackerID, skillID, damageType, damage))
		
		

