# -*- coding: utf-8 -*-
import KBEngine
from KBEDebug import * 

class CombatPropertys:
	def __init__(self):
		pass
	
	def set_HP_Max(self, oldValue):
		"""
		Property method.
		服务器设置了属性
		"""
		DEBUG_MSG("%s::set_HP_Max: %i changed:%s->%s" % (self.getScriptName(), self.id, oldValue, self.HP_Max))
		
		KBEngine.fireEvent("set_HP_Max", self.id, self.HP_Max)

	def set_HP(self, oldValue):
		"""
		Property method.
		服务器设置了属性
		"""
		DEBUG_MSG("%s::set_HP: %i changed:%s->%s" % (self.getScriptName(), self.id, oldValue, self.HP))
		
	def set_MP_Max(self, oldValue):
		"""
		Property method.
		服务器设置了属性
		"""
		DEBUG_MSG("%s::set_MP_Max: %i changed:%s->%s" % (self.getScriptName(), self.id, oldValue, self.MP_Max))
		
		KBEngine.fireEvent("set_MP_Max", self.id, self.MP_Max)

	def set_MP(self, oldValue):
		"""
		Property method.
		服务器设置了属性
		"""
		DEBUG_MSG("%s::set_MP: %i changed:%s->%s" % (self.getScriptName(), self.id, oldValue, self.MP))
