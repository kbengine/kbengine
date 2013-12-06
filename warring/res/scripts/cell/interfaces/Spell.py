# -*- coding: utf-8 -*-
import KBEngine
import skills
import GlobalConst
import wtimer
from KBEDebug import * 
import skillbases.SCObject as SCObject

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
		
	def onBuffTick(self, tid, tno):
		"""
		buff的tick
		"""
		DEBUG_MSG("onBuffTick:%i" % tid)
	
	def intonate(self, skill, scObject):
		"""
		吟唱技能
		"""
		pass
		
	def spellTarget(self, skillID, targetID):
		"""
		defined.
		对一个目标entity施放一个技能
		"""
		DEBUG_MSG("Spell::spellTarget(%i):skillID=%i, targetID=%i" % (self.id, skillID, targetID))
		
		skill = skills.getSkill(skillID)
		if skill is None:
			ERROR_MSG("Spell::spellTarget(%i):skillID=%i not found" % (self.id, skillID))
			return

		target = KBEngine.entities.get(targetID)
		if target is None:
			ERROR_MSG("Spell::spellTarget(%i):targetID=%i not found" % (self.id, targetID))
			return
		
		scobject = SCObject.createSCEntity(target)
		ret = skill.canUse(self, scobject)
		if ret != GlobalConst.GC_OK:
			ERROR_MSG("Spell::spellTarget(%i): cannot spell skillID=%i, targetID=%i, code=%i" % (self.id, skillID, targetID, ret))
			return
			
		skill.use(self, scobject)
	
	def spellPosition(self, position):
		pass
		
Spell._timermap = {}
Spell._timermap[wtimer.TIMER_TYPE_BUFF_TICK] = Spell.onBuffTick