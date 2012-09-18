# -*- coding: utf-8 -*-
import KBEngine
import skills
import GlobalConst
from KBEDebug import * 

class Spell:
	def __init__(self):
		pass

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
