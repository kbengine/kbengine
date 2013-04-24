# -*- coding: utf-8 -*-
import KBEngine
import KBExtra
from KBEDebug import *

# 模型的面向调整(由于用的现成的模型， 一些模型导出时朝向不对， 通过这个值进行调整达到统一朝向)
faceToTable = {
	"ninja" : -1,
	"Ogre"	: 1,
	"dwarf" : -1,
}

class GameObject(KBEngine.Entity):
	def __init__(self):
		KBEngine.Entity.__init__(self)

	def getScriptName(self):
		return self.__class__.__name__
		
	def enterWorld(self):
		"""
		KBEngine method.
		这个entity已经进入世界了
		"""
		pass
		
	def leaveWorld(self):
		"""
		KBEngine method.
		这个entity将要离开世界了
		"""
		pass
		
	def set_name(self, oldValue):
		"""
		Property method.
		服务器设置了name属性
		"""
		DEBUG_MSG("%s::set_name: %i changed:%s->%s" % (self.getScriptName(), self.id, oldValue, self.name))
		
		# 通知表现层改变表现
		KBEngine.fireEvent("set_name", self.id, self.name)

	def set_modelNumber(self, oldValue):
		"""
		Property method.
		服务器设置了modelNumber属性
		"""
		DEBUG_MSG("%s::set_modelNumber: %i changed:%s->%s" % (self.getScriptName(), self.id, oldValue, self.modelNumber))
		
		# 通知表现层改变表现
		KBEngine.fireEvent("set_modelNumber", self.id, self.modelNumber)
		
	def set_modelScale(self, oldValue):
		"""
		Property method.
		服务器设置了modelNumber属性
		"""
		DEBUG_MSG("%s::set_modelScale: %i changed:%s->%s" % (self.getScriptName(), self.id, oldValue, self.modelScale))
		
		KBEngine.fireEvent("set_modelScale", self.id, self.modelScale)
