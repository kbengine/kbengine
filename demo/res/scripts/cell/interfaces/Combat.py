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
		defined.
		"""
		if self.isDestroyed or self.isDead():
			return
		
		if self.HP <= damage:
			if self.canDie(attackerID, skillID, damageType, damage):
				self.die(attackerID)
		else:
			self.setHP(self.HP - damage)
	
	def addEnemy(self, entityID, enmity):
		"""
		defined.
		添加敌人
		"""
		DEBUG_MSG("%s::addEnemy: %i entity=%i, enmity=%i" % \
						(self.getScriptName(), self.id, entityID, enmity))
		
		self.enemyLog.append(entityID)
		self.onAddEnemy(entityID)
	
	def onAddEnemy(self, entityID):
		"""
		virtual method.
		有敌人进入列表
		"""
		pass
	
	def removeEnemy(self, entityID):
		"""
		defined.
		删除敌人
		"""
		DEBUG_MSG("%s::removeEnemy: %i entity=%i" % \
						(self.getScriptName(), self.id, entityID))
		
		self.enemyLog.remove(entityID)
		self.onRemoveEnemy(entityID)

	def onRemoveEnemy(self, entityID):
		"""
		virtual method.
		删除敌人
		"""
		pass
	
	def checkEnemys(self):
		"""
		检查敌人列表
		"""
		for idx in range(len(self.enemyLog) - 1, -1, -1):
			entity = KBEngine.entities.get(self.enemyLog[idx])
			if entity is None:
				self.removeEnemy(self.enemyLog[idx])

Combat._timermap = {}
