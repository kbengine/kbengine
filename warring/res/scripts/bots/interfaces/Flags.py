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

	# ----------------------------------------------------------------
	# public
	# ----------------------------------------------------------------
	def hasFlags(self, flags):
		return self.flags & flags

	# ----------------------------------------------------------------
	# callback
	# ----------------------------------------------------------------
	def onFlagsChanged_(self, oldflags, newflags):
		"""
		virtual method.
		"""
		pass
			
	# ----------------------------------------------------------------
	# property method
	# ----------------------------------------------------------------
	def set_flags(self, old):
		self.onFlagsChanged_(old, self.flags)
		


