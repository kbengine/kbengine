# -*- coding: utf-8 -*-
import KBEngine
import wtimer
import time
import random
from KBEDebug import * 

class AI:
	def __init__(self):
		pass

	def enable(self):
		"""
		激活entity
		"""
		self.heartBeatTimerID = \
		self.addTimer(random.randint(0, 1), 1, wtimer.TIMER_TYPE_HEARDBEAT)				# 心跳timer, 每1秒一次
	
	def disable(self):
		"""
		禁止这个entity做任何行为
		"""
		self.delTimer(self.heartBeatTimerID)
		self.heartBeatTimerID = 0
		
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
		self.think()
	
	def think(self):
		"""
		virtual method.
		"""
		pass
		
AI._timermap = {}
AI._timermap[wtimer.TIMER_TYPE_HEARDBEAT] = AI.onHeardTimer