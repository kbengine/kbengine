# -*- coding: utf-8 -*-
import random
import math
import time
import KBEngine
import wtimer
from KBEDebug import *
from interfaces.GameObject import GameObject
from interfaces.Combat import Combat
from interfaces.Spell import Spell

class Monster(GameObject, Combat, Spell):
	def __init__(self):
		GameObject.__init__(self)
		Combat.__init__(self) 
		Spell.__init__(self) 
		
	def enable(self):
		"""
		激活entity
		"""
		self.heartBeatTimerID = \
		self.addTimer(random.randint(0, 1000), 1000, wtimer.TIMER_TYPE_HEARDBEAT)				# 心跳timer, 每1秒一次
	
	def disable(self):
		"""
		禁止这个entity做任何行为
		"""
		self.delTimer(self.heartBeatTimerID)
		self.heartBeatTimerID = 0
		
	def onEnterTrap(self, entity, range, trapID):
		"""
		KBEngine method.
		一个entity进入了自己所投放的某个陷阱区域
		@param entity	: 被捕获到的entity
		@param range	: 被捕获到的entity离自身的距离
		@param trapID	: 这个陷阱的唯一ID addProximity的返回值
		"""
		if entity.__class__.__name__ == "Avatar":
			DEBUG_MSG("entity:%s %d enterTrap. trapID=%i, range=%f" % (entity.__class__.__name__, entity.id, trapID, range))
	
	def onLeaveTrap(self, entity, range, trapID):
		"""
		KBEngine method.
		一个entity离开了自己所投放的某个陷阱区域
		@param entity	: 被捕获到的entity
		@param range	: 被捕获到的entity离自身的距离
		@param trapID	: 这个陷阱的唯一ID addProximity的返回值
		"""
		if entity.__class__.__name__ == "Avatar":
			DEBUG_MSG("entity:%s %d leaveTrap. trapID=%i, range=%f" % (entity.__class__.__name__, entity.id, trapID, range))
			
	def onWitnessed(self, isWitnessed):
		"""
		KBEngine method.
		此实体是否被观察者(player)观察到, 此接口主要是提供给服务器做一些性能方面的优化工作，
		在通常情况下，一些entity不被任何客户端所观察到的时候， 他们不需要做任何工作， 利用此接口
		可以在适当的时候激活或者停止这个entity的任意行为。
		@param isWitnessed	: 为false时， entity脱离了任何观察者的观察
		"""
		DEBUG_MSG("%s::onWitnessed: %i isWitnessed=%i." % (self.getScriptName(), self.id, isWitnessed))
		
		if isWitnessed:
			self.enable()
		else:
			self.disable()
			
	def onHeardTimer(self, tid, tno):
		"""
		entity的心跳
		"""
		if not self.isMoving:
			if self.moveWaitCount == 0:
				self.randomWalk()
			else:
				self.moveWaitCount -= 1
		
	def randomWalk(self):
		"""
		随机移动entity
		"""
		rnd = random.random()
		a = 30.0 * rnd				# 移动半径距离在30米内
		b = 360.0 * rnd				# 随机一个角度
		x = a * math.cos( b ) 		# 半径 * 正余玄
		z = a * math.sin( b )
		self.moveToPoint((self.spawnPos.x + x, self.spawnPos.y, self.spawnPos.z + z), self.moveSpeed, 1, True, True)
		self.isMoving = True
		self.moveWaitCount = 0
		
	def onMove(self, userData):
		"""
		KBEngine method.
		使用引擎的任何移动相关接口， 在entity一次移动完成时均会调用此接口
		"""
		self.moveWaitCount = random.randint(5, 15)
		self.isMoving = False

Monster._timermap = {}
Monster._timermap.update(GameObject._timermap)
Monster._timermap.update(Combat._timermap)
Monster._timermap.update(Spell._timermap)
Monster._timermap[wtimer.TIMER_TYPE_HEARDBEAT] = Monster.onHeardTimer
