# -*- coding: utf-8 -*-
import KBEngine
import KBExtra
from KBEDebug import * 

class Motion:
	def __init__(self):
		self.set_moveSpeed(0)

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
		DEBUG_MSG("%s::onMoveFailure: %i controllerId =%i, userarg=%s" % \
						(self.getScriptName(), self.id, controllerId, userarg))
		
	def onMoveOver(self, controllerId, userarg):
		"""
		KBEngine method.
		使用引擎的任何移动相关接口， 在entity移动结束时均会调用此接口
		"""
		#DEBUG_MSG("%s::onMoveOver: %i controllerId =%i, userarg=%s" % \
		#				(self.getScriptName(), self.id, controllerId, userarg))
		pass

	def set_moveSpeed(self, oldValue):
		"""
		Property method.
		服务器设置了moveSpeed属性
		"""
		DEBUG_MSG("%s::set_moveSpeed: %i changed:%s->%s" % (self.getScriptName(), self.id, oldValue, self.moveSpeed))

		# 设置引擎层entity移动速度
		self.velocity = self.moveSpeed * 0.1

