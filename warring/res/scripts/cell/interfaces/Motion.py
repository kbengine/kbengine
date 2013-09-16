# -*- coding: utf-8 -*-
import KBEngine
import math
import time
import random
from KBEDebug import * 

class Motion:
	def __init__(self):
		pass
	
	def stopMotion(self):
		"""
		停止移动
		"""
		if self.isMoving:
			#INFO_MSG("%i stop motion." % self.id)
			self.cancel("Movement")
			self.isMoving = False
			
	def onMove(self, controllerId, userarg):
		"""
		KBEngine method.
		使用引擎的任何移动相关接口， 在entity一次移动完成时均会调用此接口
		"""
		#DEBUG_MSG("%s::onMove: %i controllerId =%i, userarg=%s" % \
		#				(self.getScriptName(), self.id, controllerId, userarg))
		self.isMoving = True
		
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
		self.gotoPosition((basePos.x + x, basePos.y, basePos.z + z))
		self.isMoving = True
		self.nextMoveTime = int(time.time() + random.randint(5, 15))

	def backSpawnPos(self):
		"""
		virtual method.
		"""
		INFO_MSG("%s::backSpawnPos: %i, pos=%s, speed=%f." % \
			(self.getScriptName(), self.id, self.spawnPos, self.moveSpeed))
			
		self.gotoPosition(self.spawnPos)
	
	def gotoEntity(self, targetID, dist = 0.0):
		"""
		virtual method.
		移动到entity位置
		"""
		if self.isMoving:
			self.stopMotion()
		
		entity = KBEngine.entities.get(targetID)
		if entity is None:
			DEBUG_MSG("%s::gotoEntity: not found targetID=%i" % (targetID))
			return
			
		if entity.position.distTo(self.position) <= dist:
			return
			
		self.isMoving = True
		self.moveToEntity(targetID, self.moveSpeed, dist, 1, True, True)
		
	def gotoPosition(self, position):
		"""
		virtual method.
		移动到位置
		"""
		if self.isMoving:
			self.stopMotion()

		if self.position.distTo(position) <= 0.05:
			return
			
		self.isMoving = True
		self.moveToPoint(tuple(position), self.moveSpeed, 1, True, True)
		
Motion._timermap = {}
