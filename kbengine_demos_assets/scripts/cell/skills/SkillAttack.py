# -*- coding: utf-8 -*-
import KBEngine
import random
from KBEDebug import * 
from skills.base.SkillInitiative import SkillInitiative

class SkillAttack(SkillInitiative):
	def __init__(self):
		SkillInitiative.__init__(self)

	def canUse(self, caster, scObject):
		"""
		virtual method.
		可否使用 
		@param caster: 使用技能者
		@param receiver: 受技能影响者
		"""
		return SkillInitiative.canUse(self, caster, scObject)
		
	def use(self, caster, scObject):
		"""
		virtual method.
		使用技能
		@param caster: 使用技能者
		@param receiver: 受技能影响者
		"""
		return SkillInitiative.use(self, caster, scObject)
		
	def receive(self, caster, receiver):
		"""
		virtual method.
		可以对受术者做一些事情了
		"""
		if self.getID() == 1:
			receiver.recvDamage(caster.id, self.getID(), 0, random.randint(0, 10))
		elif self.getID() == 1000101:
			#caster.position = caster.getStopPoint(caster.yaw, 15.0)
			receiver.recvDamage(caster.id, self.getID(), 0, random.randint(0, 100))
		elif self.getID() == 2000101:
			receiver.recvDamage(caster.id, self.getID(), 0, random.randint(0, 100))
		elif self.getID() == 3000101:
			receiver.recvDamage(caster.id, self.getID(), 0, random.randint(0, 100))
		elif self.getID() == 4000101:
			receiver.recvDamage(caster.id, self.getID(), 0, random.randint(0, 100))
		elif self.getID() == 5000101:
			receiver.recvDamage(caster.id, self.getID(), 0, random.randint(0, 100))
		elif self.getID() == 6000101:
			receiver.recvDamage(caster.id, self.getID(), 0, random.randint(0, 100))
		elif self.getID() == 7000101:
			receiver.recvDamage(caster.id, self.getID(), 0, random.randint(0, 10))