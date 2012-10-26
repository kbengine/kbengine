# -*- coding: utf-8 -*-
import KBEngine
import skills
import GlobalConst
import wtimer
from KBEDebug import * 

class Spell:
	def __init__(self):
		#self.addTimer(1,1,wtimer.TIMER_TYPE_BUFF_TICK)
		pass
		
	def addDBuff(self, buffData):
		"""
		defined method.
		添加buff
		"""
		pass

	def removeDBuff(self, buffData):
		"""
		defined method.
		删除buff
		"""
		pass
		
	def onBuffTick(self, tid):
		"""
		buff的tick
		"""
		DEBUG_MSG("onBuffTick:%i" % tid)
		
	def spellTarget(self, srcEntityID, skillID, targetID):
		"""
		exposed.
		对一个目标entity施放一个技能
		"""
		if srcEntityID != self.id:
			return
			 
		DEBUG_MSG("Spell::spellTarget(%i):skillID=%i, srcEntityID=%i, targetID=%i" % (self.id, skillID, srcEntityID, targetID))
		
		skill = skills.getSkill(skillID)
		if skill is None:
			ERROR_MSG("Spell::spellTarget(%i):skillID=%i not found" % (self.id, skillID))
			return

		target = KBEngine.entities.get(targetID)
		if target is None:
			ERROR_MSG("Spell::spellTarget(%i):targetID=%i not found" % (self.id, targetID))
			return

		ret = skill.canUse(self, target)
		if ret != GlobalConst.GC_OK:
			ERROR_MSG("Spell::spellTarget(%i): cannot spell skillID=%i, targetID=%i, code=%i" % (self.id, skillID, targetID, ret))
			return
			
		skill.use(self, target)

Spell._timermap = {}
Spell._timermap[wtimer.TIMER_TYPE_BUFF_TICK] = Spell.onBuffTick