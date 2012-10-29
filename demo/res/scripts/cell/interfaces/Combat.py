# -*- coding: utf-8 -*-
import KBEngine
from KBEDebug import * 
from interfaces.CombatPropertys import CombatPropertys

class Combat(CombatPropertys):
	def __init__(self):
		CombatPropertys.__init__(self)
		
	def canUpgrade(self):
		"""
		virtual method.
		"""
		return True
		
	def upgrade(self):
		"""
		for real
		"""
		if self.canUpgrade():
			self.addLevel(1)
			
	def addLevel(self, lv):
		"""
		for real
		"""
		self.level += lv
		self.onLevelChanged(lv)
		
	def onLevelChanged(self, addlv):
		"""
		virtual method.
		"""
		pass
		
	def isDead(self):
		"""
		"""
		return self.state == scdefine.ENTITY_STATE_DEAD
		
	def die(self, killerID):
		"""
		"""
		if self.isDestroyed or self.isDead():
			return
		
		if killerID == self.id:
			killerID = 0
			
		INFO_MSG("%i i die. killerID:%i." % (self.id, killerID))
		killer = KBEngine.entities.get(killerID)
		if killer:
			killer.onKiller(self.id)
			
		self.onBeforeDie(killerID)
		self.onDie(killerID)
		self.changeState(scdefine.ENTITY_STATE_DEAD)
		self.onAfterDie(killerID)
		
	def onDie(self, killerID):
		"""
		virtual method.
		"""
		self.setHP(0)
		self.setMP(0)

	def onBeforeDie(self, killerID):
		"""
		virtual method.
		"""
		pass

	def onAfterDie(self, killerID):
		"""
		virtual method.
		"""
		pass
	
	def onKiller(self, entityID):
		"""
		defined.
		我击杀了entity
		"""
		pass
		
	def recvDamage(self, attackerID, skillID, damageType, damage):
		"""
		defined
		"""
		if self.isDestroyed or self.isDead():
			return
		
		if self.HP <= damage:
			if self.canDie(attackerID, skillID, damageType, damage):
				self.die(attackerID)
		else:
			self.setHP(self.HP - damage)
			
	def commitFight(self, exposed, targetID, skillID):
		"""
		Exposed method.
		"""
		DEBUG_MSG("Combat::commitFight: targetID=%i, skillID=%i" % (targetID, skillID))
		self.getCurrSpace().commitFight(self.id, targetID, skillID)
		
Combat._timermap = {}
