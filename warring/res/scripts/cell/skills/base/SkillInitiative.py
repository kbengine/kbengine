# -*- coding: utf-8 -*-
import KBEngine
import random
import GlobalConst
from KBEDebug import * 
from skillbases.SObject import SObject
from skillbases.SCObject import SCObject

class SkillInitiative(SObject):
	def __init__(self):
		SObject.__init__(self)
		
	def loadFromDict(self, dictDatas):
		"""
		virtual method.
		从字典中创建这个对象
		"""
		SObject.loadFromDict(self, dictDatas)
		
		# 法术速度
		self.speed = dictDatas.get('speed', 0)
		
		# 吟唱时间
		self.intonateTime = dictDatas.get("intonateTime", 0.0)
		
		# 最小最大施放范围
		self.rangeMin = dictDatas.get('rangeMin', 0)
		self.rangeMax = dictDatas.get('rangeMax', 2)
		self.__castMaxRange = dictDatas.get("rangeMaxAdd", 10.0)
		
		# 施法转向
		self.__isRotate	= dictDatas.get("isRotate", True)
		
		# 最大受术个数
		self.maxReceiveCount = dictDatas.get("maxReceiverCount", 999)
		
		# cd
		self.limitCDs = dictDatas.get("limitCDs", [1])
		self.springCDs = dictDatas.get("springCDs", [])
		
	def getRangeMin(self, caster):
		"""
		virtual method.
		"""
		return self.rangeMin

	def getRangeMax(self, caster):
		"""
		virtual method.
		"""
		return self.rangeMax
		
	def getIntonateTime(self, caster):
		"""
		virtual method.
		"""
		return self.intonateTime
		
	def getCastMaxRange(self, caster):
		return self.getRangeMax(caster) + self.__castMaxRange

	def getSpeed(self):
		return self.speed

	def isRotate(self):
		return self.__isRotate

	def getMaxReceiverCount(self):
		return self.maxReceiverCount

	def canUse(self, caster, scObject):
		"""
		virtual method.
		可否使用 
		@param caster: 使用技能者
		@param receiver: 受技能影响者
		"""
		return GlobalConst.GC_OK
		
	def use(self, caster, scObject):
		"""
		virtual method.
		使用技能
		@param caster: 使用技能者
		@param receiver: 受技能影响者
		"""
		self.cast(caster, scObject)
		return GlobalConst.GC_OK
		
	def cast(self, caster, scObject):
		"""
		virtual method.
		施放技能
		"""
		delay = self.distToDelay(caster, scObject)
		#INFO_MSG("%i cast skill[%i] delay=%s." % (caster.id, self.id, delay))
		if delay <= 0.1:
			self.onArrived(caster, scObject)
		else:
			#INFO_MSG("%i add castSkill:%i. delay=%s." % (caster.id, self.id, delay))
			caster.addCastSkill(self, scObject, delay)

		self.onSkillCastOver_(caster, scObject)
		
	def distToDelay(self, caster, scObject):
		"""
		"""
		return scObject.distToDelay(self.getSpeed(), caster.position)
		
	def onArrived(self, caster, scObject):
		"""
		virtual method.
		到达了目标
		"""
		self.receive(caster, scObject.getObject())
		
	def receive(self, caster, receiver):
		"""
		virtual method.
		可以对受术者做一些事情了
		"""
		pass

	def onSkillCastOver_(self, caster, scObject):
		"""
		virtual method.
		法术施放完毕通知
		"""
		pass
