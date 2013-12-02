# -*- coding: utf-8 -*-
import KBEngine
import KBExtra
from KBEDebug import * 

class Motion:
	def __init__(self):
		self.set_moveSpeed(0)

	def onMoveToPoint(self, destination, velocity, faceMovement, moveVertically):
		"""
		KBEngine method.
		这个entity将要移动到某个点， 由服务器通知
		"""
		KBExtra.moveToPoint(self.id, destination, velocity, faceMovement, moveVertically)
		
	def set_moveSpeed(self, oldValue):
		"""
		Property method.
		服务器设置了moveSpeed属性
		"""
		DEBUG_MSG("%s::set_moveSpeed: %i changed:%s->%s" % (self.getScriptName(), self.id, oldValue, self.moveSpeed))

		# 设置引擎层entity移动速度
		self.velocity = self.moveSpeed * 0.1

