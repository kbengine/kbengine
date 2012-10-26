# -*- coding: utf-8 -*-
import KBEngine
import KBExtend
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

	def enterWorld(self):
		"""
		KBEngine method.
		这个entity已经进入世界了
		"""
		# 通知APP创建一个entity
		KBExtend.createEntity(self.id, self.name, tuple(self.position), tuple(self.direction), \
								self.modelNumber, self.modelScale, self.moveSpeed, self.modelYOffset, faceToTable[self.modelNumber])
		
	def leaveWorld(self):
		"""
		KBEngine method.
		这个entity将要离开世界了
		"""
		KBExtend.destroyEntity(self.id)
		
	def onMoveToPoint(self, destination, velocity, faceMovement, moveVertically):
		"""
		KBEngine method.
		这个entity将要移动到某个点， 由服务器通知
		"""
		KBExtend.moveToPoint(self.id, destination, velocity, faceMovement, moveVertically)
		
	def set_name(self, oldValue):
		"""
		Property method.
		服务器设置了name属性
		"""
		DEBUG_MSG("服务器设置了name属性:", oldValue, "改变为", self.name)
		# 通知表现层改变表现
		KBExtend.setEntityName(self.id, self.name)

	def set_moveSpeed(self, oldValue):
		"""
		Property method.
		服务器设置了moveSpeed属性
		"""
		DEBUG_MSG("服务器设置了moveSpeed属性:", oldValue, "改变为", self.moveSpeed)
		# 设置渲染层entity的速度
		KBExtend.setMoveSpeed(self.id, self.moveSpeed)

	def set_modelNumber(self, oldValue):
		"""
		Property method.
		服务器设置了modelNumber属性
		"""
		DEBUG_MSG("服务器设置了modelNumber属性:", oldValue, "改变为", self.modelNumber)
		KBExtend.setModel(self.id, self.modelNumber)
		
	def set_modelScale(self, oldValue):
		"""
		Property method.
		服务器设置了modelNumber属性
		"""
		DEBUG_MSG("服务器设置了modelScale属性:", oldValue, "改变为", self.modelScale)
		KBExtend.scaleModel(self.id, self.modelScale)
		
	def set_modelYOffset(self, oldValue):
		"""
		Property method.
		服务器设置了name属性
		"""
		DEBUG_MSG("服务器设置了modelYOffset属性:", oldValue, "改变为", self.modelYOffset)
		# 通知表现层改变表现
		KBExtend.setModelYOffset(self.id, self.modelYOffset)