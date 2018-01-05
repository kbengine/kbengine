# -*- coding: utf-8 -*-
import KBEngine
from KBEDebug import * 
from skillbases.SObject import SObject

class DBuff(SObject):
	def __init__(self):
		SObject.__init__(self)

		self._loopTime = 0		# 周期触发时间
		self._totalTime = 0		# 持续时间
		
	def loadFromDict(self, dictDatas):
		"""
		virtual method.
		从字典中创建这个对象
		"""
		SObject.loadFromDict(self, dictDatas)
		self._loopTime = dictDatas.get('looptime', 0)
		self._totalTime = dictDatas.get('totaltime', 0)
		
	def onLoopTrigger(self, context):
		"""
		virtual method.
		周期触发
		@param context: buff/debuff上下文
		"""
		pass
		
	def onAttach(self, context):
		"""
		virtual method.
		buff/debuff被绑定时
		@param context: buff/debuff上下文
		"""
		pass
		
	def onDetach(self, context):
		"""
		virtual method.
		buff/debuff取消绑定时
		@param context: buff/debuff上下文
		"""
		pass
