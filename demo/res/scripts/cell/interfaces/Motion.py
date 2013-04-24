# -*- coding: utf-8 -*-
import KBEngine
import math
import time
import random
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
		
		self.isMoving = False
		
	def onMoveOver(self, controllerId, userarg):
		"""
		KBEngine method.
		使用引擎的任何移动相关接口， 在entity移动结束时均会调用此接口
		"""
		self.isMoving = False
		
	def randomWalk(self, basePos):
		"""
		随机移动entity
		"""
		if self.isMoving:
			return False
			
		if time.time() < self.nextMoveTime:
			return False
			
		rnd = random.random()
		a = 30.0 * rnd				# 移动半径距离在30米内
		b = 360.0 * rnd				# 随机一个角度
		x = a * math.cos( b ) 		# 半径 * 正余玄
		z = a * math.sin( b )
		self.moveToPoint((basePos.x + x, basePos.y, basePos.z + z), self.moveSpeed, 1, True, True)
		self.isMoving = True
		self.nextMoveTime = int(time.time() + random.randint(5, 15))
		
Motion._timermap = {}
