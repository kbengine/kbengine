# -*- coding: utf-8 -*-
#
"""
"""

import GlobalDefine
from KBEDebug import *

class State:
	"""
	"""
	def __init__(self):
		"""
		"""
		pass

	# ----------------------------------------------------------------
	# public
	# ----------------------------------------------------------------
	def getState(self):
		return self.state

	def isState(self, state):
		return self.state == state

	def isForbid(self, forbid):
		"""
		scdefine.FORBID_***
		"""
		return self.forbids & forbid

	# ----------------------------------------------------------------
	# callback
	# ----------------------------------------------------------------
	def onStateChanged_(self, oldState, newState):
		"""
		virtual method.
		"""
		# 通知表现层改变表现
		pass

	def onForbidChanged_(self, oldForbids, newForbids):
		"""
		virtual method.
		"""
		pass

	# ----------------------------------------------------------------
	# property method
	# ----------------------------------------------------------------
	def set_state(self, oldValue):
		DEBUG_MSG("%s::set_state: %i changed:%s->%s" % (self.getScriptName(), self.id, oldValue, self.state))
		self.onStateChanged_(oldValue, self.state)

	def set_effStates(self, oldValue):
		DEBUG_MSG("%s::set_effStates: %i changed:%s->%s" % (self.getScriptName(), self.id, oldValue, self.effStates))
		self.onEffectStateChanged_(oldValue, self.effStates)

	def set_forbids(self, oldValue):
		DEBUG_MSG("%s::set_forbids: %i changed:%s->%s" % (self.getScriptName(), self.id, oldValue, self.forbids))
		self.onForbidChanged_(oldValue, self.forbids)
