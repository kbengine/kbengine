# -*- coding: utf-8 -*-
#
"""
处理entity的一些状态
"""
import GlobalDefine
from KBEDebug import *

class State:
	"""
	"""
	def __init__(self):
		self._forbidCounter = [0] * len(GlobalDefine.FORBID_ALL)
		self.forbidCounterInc(GlobalDefine.FORBID_ACTIONS[self.state])

	def initEntity(self):
		"""
		virtual method.
		"""
		pass
		
	def isState(self, state):
		return self.state == state
	
	def isSubState(self, state):
		return self.subState == state
	
	def isForbid(self, forbid):
		return self.forbids & forbid
	
	def getState(self):
		return self.state

	def getSubState(self):
		return self.subState

	def getForbidCounter(self, forbid):
		"""
		获取禁止计数器的数据
		"""
		return self._forbidCounter[GlobalDefine.FORBID_ALL.index(forbid)]
					
	def changeSubState(self, subState):
		"""
		改变当前子状态
		GlobalDefine.ENTITY_SUB_STATE_**
		"""
		if self.subState != subState:
			oldSubState = self.subState
			self.subState = subState
			self.onSubStateChanged_(oldSubState, self.subState)

	def forbidCounterInc(self, forbids):
		"""
		禁止计数器加一
		"""
		fbList = []
		for i, fb in enumerate(GlobalDefine.FORBID_ALL):
			if forbids & fb:
				if self._forbidCounter[i] == 0:
					fbList.append(fb)
				self._forbidCounter[i] += 1

		# kbe 任何时候对定义属性赋值都会产生事件
		if len(fbList) > 0:
			self.forbids |= forbids
			for fb in fbList:
				self.onForbidChanged_(fb, True)

	def forbidCounterDec(self, forbids):
		"""
		禁止计数器减一
		"""
		fbList = []
		for i, fb in enumerate(GlobalDefine.FORBID_ALL):
			if forbids & fb:
				self._forbidCounter[i] -= 1
				if self._forbidCounter[i] == 0:
					fbList.append(fb)

		# kbe 任何时候对定义属性赋值都会产生事件
		if len(fbList) > 0:
			self.forbids &= ~forbids
			for fb in fbList:
				self.onForbidChanged_(fb, False)
		
	#--------------------------------------------------------------------------------------------
	#                              Callbacks
	#--------------------------------------------------------------------------------------------
	def onForbidChanged_(self, forbid, isInc):
		"""
		virtual method.
		entity禁止 条件改变
		@param isInc		:	是否是增加
		"""
		pass

	def onStateChanged_(self, oldstate, newstate):
		"""
		virtual method.
		entity状态改变了
		"""
		self.changeSubState(GlobalDefine.ENTITY_SUB_STATE_NORMAL)
		INFO_MSG("%s:onStateChanged_: %i oldstate=%i to newstate=%i, forbids=%s, subState=%i." % (self.getScriptName(), \
				self.id, oldstate, newstate, self._forbidCounter, self.subState))

	def onSubStateChanged_(self, oldSubState, newSubState):
		"""
		virtual method.
		子状态改变了
		"""
		#INFO_MSG("%i oldSubstate=%i to newSubstate=%i" % (self.id, oldSubState, newSubState))
		pass
		
	#--------------------------------------------------------------------------------------------
	#                              defined
	#--------------------------------------------------------------------------------------------
	def changeState(self, state):
		"""
		defined
		改变当前主状态
		GlobalDefine.ENTITY_STATE_**
		"""
		if self.state != state:
			oldstate = self.state
			self.state = state
			self.forbidCounterDec(GlobalDefine.FORBID_ACTIONS[oldstate])
			self.forbidCounterInc(GlobalDefine.FORBID_ACTIONS[state])
			self.onStateChanged_(oldstate, state)


