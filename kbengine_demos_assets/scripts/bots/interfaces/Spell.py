# -*- coding: utf-8 -*-
import KBEngine
import skills
import GlobalConst
from KBEDebug import * 

class Spell:
	def __init__(self):
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
		
	def spellTarget(self, skillID, targetID):
		"""
		对一个目标entity施放一个技能
		"""
		self.cell.useTargetSkill(skillID, targetID)

