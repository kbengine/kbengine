# -*- coding: utf-8 -*-
import KBEngine
import GlobalConst
from KBEDebug import * 
from skillbases.SObject import SObject

class SkillInitiative(SObject):
	def __init__(self):
		SObject.__init__(self)

	def canUse(self, caster, receiver):
		"""
		virtual method.
		可否使用 
		@param caster: 使用技能者
		@param receiver: 受技能影响者
		"""
		return GlobalConst.GC_OK
		
	def use(self, caster, receiver):
		"""
		virtual method.
		使用技能
		@param caster: 使用技能者
		@param receiver: 受技能影响者
		"""
		pass
		
	def cast(self, caster, receiver):
		"""
		virtual method.
		施放技能
		"""
		pass
		
	def do(self, caster, receiver):
		"""
		virtual method.
		技能已经到达目标
		可以做一些事情了
		"""
		pass