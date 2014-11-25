# -*- coding: utf-8 -*-
import KBEngine
import math
import Math
import time
import random
from KBEDebug import * 

class Motion:
	"""
	移动相关的封装
	"""
	def __init__(self):
		pass
	
	def stopMotion(self):
		"""
		停止移动
		"""
		if self.isMoving:
			#INFO_MSG("%i stop motion." % self.id)
			self.cancelController("Movement")
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
		
		while True:
			rnd = random.random()
			a = 30.0 * rnd				# 移动半径距离在30米内
			b = 360.0 * rnd				# 随机一个角度
			x = a * math.cos( b ) 		# 半径 * 正余玄
			z = a * math.sin( b )
			
			destPos = (basePos.x + x, basePos.y, basePos.z + z)
			
			if self.position.distTo(destPos) < 2.0:
				continue
				
			self.gotoPosition(destPos)
			self.isMoving = True
			self.nextMoveTime = int(time.time() + random.randint(5, 15))
			break

	def resetSpeed(self):
		walkSpeed = self.getDatas()["moveSpeed"]
		if walkSpeed != self.moveSpeed:
			self.moveSpeed = walkSpeed
				
	def backSpawnPos(self):
		"""
		virtual method.
		"""
		INFO_MSG("%s::backSpawnPos: %i, pos=%s, speed=%f." % \
			(self.getScriptName(), self.id, self.spawnPos, self.moveSpeed * 0.1))
		
		self.resetSpeed()
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
		self.moveToEntity(targetID, self.moveSpeed * 0.1, dist, None, True, 1)
		
	def gotoPosition(self, position, dist = 0.0):
		"""
		virtual method.
		移动到位置
		"""
		if self.isMoving:
			self.stopMotion()

		if self.position.distTo(position) <= 0.05:
			return

		self.isMoving = True
		speed = self.moveSpeed * 0.1
		
		if self.canNavigate():
			self.navigate(Math.Vector3(position), speed, dist, speed, 512.0, 1, 0.5, None)
		else:
			if dist > 0.0:
				destPos = Math.Vector3(position) - self.position
				destPos.normalise()
				destPos *= dist
				destPos = position - destPos
			else:
				destPos = Math.Vector3(position)
			
			self.moveToPoint(destPos, speed, None, 1, 1)

	def getStopPoint(self, yaw = None, rayLength = 100.0):
		"""
		"""
		if yaw is None:yaw = self.yaw
		yaw = (yaw / 2);
		vv = Math.Vector3(math.sin(yaw), 0, math.cos(yaw))
		vv.normalise()
		vv *= rayLength
		
		lastPos = self.position + vv;
		
		pos = self.raycast(self.position, vv)
		if pos == None:
			pos = lastPos
			
		return pos
		
Motion._timermap = {}
