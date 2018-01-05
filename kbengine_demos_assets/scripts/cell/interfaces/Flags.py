# -*- coding: utf-8 -*-
#
"""
"""

import GlobalDefine
from KBEDebug import *

class Flags:
	"""
	"""
	def __init__(self):
		pass

	def setFlags(self, flags):
		self.flags = flags
		self.onFlagsChanged_(flags, True)
	
	def hasFlags(self, flags):
		return self.flags & flags

	def addFlags(self, flags):
		"""
		"""
		self.flags |= flags
		self.onFlagsChanged_(flags, True)

	def removeFlags(self, flags):
		"""
		"""
		self.flags &= ~flags
		self.onFlagsChanged_(flags, False)

	#--------------------------------------------------------------------------------------------
	#                              Callbacks
	#--------------------------------------------------------------------------------------------
	def onFlagsChanged_(self, flags, isInc):
		"""
		virtual method.
		"""
		pass


