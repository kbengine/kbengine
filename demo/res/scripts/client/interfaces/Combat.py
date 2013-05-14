# -*- coding: utf-8 -*-
import KBEngine
from KBEDebug import * 
from interfaces.CombatPropertys import CombatPropertys

class Combat(CombatPropertys):
	def __init__(self):
		CombatPropertys.__init__(self)

	def recvDamage(self, attackerID, skillID, damageType, damage):
		"""
		defined.
		"""
		DEBUG_MSG("%s::recvDamage: %i attackerID=%i, skillID=%i, damageType=%i, damage=%i" % \
			(self.getScriptName(), self.id, attackerID, skillID, damageType, damage))
		
		# 通知表现层改变表现
		KBEngine.fireEvent("recvDamage", self.id, attackerID, skillID, damageType, damage)
		

