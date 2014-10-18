# -*- coding: utf-8 -*-
import random
import math
import time
import wtimer
import d_spaces
import KBEngine
from KBEDebug import *
from interfaces.GameObject import GameObject

class Gate(KBEngine.Entity, GameObject):
	def __init__(self):
		KBEngine.Entity.__init__(self)
		GameObject.__init__(self) 
		
		self.addTimer(1, 0, wtimer.TIMER_TYPE_HEARDBEAT)				# 心跳timer, 每1秒一次

	# ----------------------------------------------------------------
	# callback
	# ----------------------------------------------------------------
	def onHeardTimer(self, tid, tno):
		"""
		entity的心跳
		"""
		self.addProximity(5.0, 0, 0)
		
	def onEnterTrap(self, entityEntering, range_xz, range_y, controllerID, userarg):
		"""
		KBEngine method.
		有entity进入trap
		"""
		if entityEntering.isDestroyed or entityEntering.getScriptName() != "Avatar":
			return
			
		DEBUG_MSG("%s::onEnterTrap: %i entityEntering=(%s)%i, range_xz=%s, range_y=%s, controllerID=%i, userarg=%i" % \
						(self.getScriptName(), self.id, entityEntering.getScriptName(), entityEntering.id, \
						range_xz, range_y, controllerID, userarg))
		
		if self.uid == 40001003: # currspace - teleport
			spaceData = d_spaces.datas.get(entityEntering.spaceUType)
			entityEntering.teleport(None, spaceData["spawnPos"], (0, 0, 0))		
		else:					 # teleport to xxspace
			if entityEntering.spaceUType == 3:
				gotoSpaceUType = 4
			else:
				gotoSpaceUType = 3
			
			spaceData = d_spaces.datas.get(gotoSpaceUType)
			entityEntering.teleportSpace(gotoSpaceUType, spaceData["spawnPos"], (0, 0, 0), {})

	def onLeaveTrap(self, entityLeaving, range_xz, range_y, controllerID, userarg):
		"""
		KBEngine method.
		有entity离开trap
		"""
		if entityLeaving.isDestroyed or entityLeaving.getScriptName() != "Avatar":
			return
			
		INFO_MSG("%s::onLeaveTrap: %i entityLeaving=(%s)%i." % (self.getScriptName(), self.id, \
				entityLeaving.getScriptName(), entityLeaving.id))
				
Gate._timermap = {}
Gate._timermap.update(GameObject._timermap)
Gate._timermap[wtimer.TIMER_TYPE_HEARDBEAT] = Gate.onHeardTimer