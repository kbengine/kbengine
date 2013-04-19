# -*- coding: utf-8 -*-
import KBEngine
from KBEDebug import * 

class Motion:
	def __init__(self):
		pass

	def onMove(self, controllerId, userarg):
		"""
		KBEngine method.
		使用引擎的任何移动相关接口， 在entity一次移动完成时均会调用此接口
		"""
		#DEBUG_MSG("%s::onMove: %i controllerId =%i, userarg=%s" % \
		#				(self.getScriptName(), self.id, controllerId, userarg))
		pass
		
	def onMoveFailure(self, controllerId, userarg):
		"""
		KBEngine method.
		使用引擎的任何移动相关接口， 在entity一次移动完成时均会调用此接口
		"""
		DEBUG_MSG("%s::onMove: %i controllerId =%i, userarg=%s" % \
						(self.getScriptName(), self.id, controllerId, userarg))
						
Motion._timermap = {}
