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
from interfaces.Motion import Motion
from interfaces.AI import AI

class Monster(GameObject, Motion, Combat, Spell, AI):
	def __init__(self):
		GameObject.__init__(self)
		Motion.__init__(self) 
		Combat.__init__(self) 
		Spell.__init__(self) 
		AI.__init__(self) 

	def think(self):
		"""
		virtual method.
		"""
		self.randomWalk(self.spawnPos)

	def onWitnessed(self, isWitnessed):
		"""
		KBEngine method.
		此实体是否被观察者(player)观察到, 此接口主要是提供给服务器做一些性能方面的优化工作，
		在通常情况下，一些entity不被任何客户端所观察到的时候， 他们不需要做任何工作， 利用此接口
		可以在适当的时候激活或者停止这个entity的任意行为。
		@param isWitnessed	: 为false时， entity脱离了任何观察者的观察
		"""
		AI.onWitnessed(self, isWitnessed)
		
Monster._timermap = {}
Monster._timermap.update(GameObject._timermap)
Monster._timermap.update(Motion._timermap)
Monster._timermap.update(Combat._timermap)
Monster._timermap.update(Spell._timermap)
Monster._timermap.update(AI._timermap)
