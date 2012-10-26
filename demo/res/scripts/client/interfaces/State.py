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
		pass

	def onForbidChanged_(self, oldForbids, newForbids):
		"""
		virtual method.
		"""
		pass

	# ----------------------------------------------------------------
	# property method
	# ----------------------------------------------------------------
	def set_state(self, old):
		self.onStateChanged_(old, self.state)

	def set_effStates(self, old):
		self.onEffectStateChanged_(old, self.effStates)

	def set_forbids(self, old):
		self.onForbidChanged_(old, self.forbids)