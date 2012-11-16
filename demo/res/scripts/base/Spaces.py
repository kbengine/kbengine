# -*- coding: utf-8 -*-
import KBEngine
import Functor
from KBEDebug import *
from interfaces.GameObject import GameObject
from SpaceAlloc import *
import d_spaces
import wtimer

class Spaces(GameObject):
	def __init__(self):
		GameObject.__init__(self)
		self._spaceAllocs = {}
		self.addTimer(3, 1, wtimer.TIMER_TYPE_CREATE_SPACES)
		self.initAlloc()
		KBEngine.globalData["SpaceMgr"] = self
	
	def initAlloc(self):
		self._tmpDatas = list(d_spaces.datas.keys())
		for utype in self._tmpDatas:
			spaceData = d_spaces.datas.get(utype)
			if spaceData["entityType"] == "SpaceCopy":
				self._spaceAllocs[utype] = SpaceAllocCopy(utype)
			elif spaceData["entityType"] == "SpaceFightCopy":
				self._spaceAllocs[utype] = SpaceAllocCopy(utype)
			else:
				self._spaceAllocs[utype] = SpaceAlloc(utype)
				
	def createSpaceOnTimer(self, tid, tno):
		"""
		创建space
		"""
		if len(self._tmpDatas) > 0:
			spaceUType = self._tmpDatas.pop(0)
			self._spaceAllocs[spaceUType].init()
			
		if len(self._tmpDatas) <= 0:
			del self._tmpDatas
			self.delTimer(tid)
			
	def loginToSpace(self, avatarEntity, spaceUType):
		"""
		define method.
		某个玩家请求登陆到某个space中
		"""
		self._spaceAllocs[spaceUType].loginToSpace(avatarEntity)
		
	def teleportSpace(self, entityMailbox, spaceUType, position, direction, context):
		"""
		define method.
		请求进入某个space中
		"""
		self._spaceAllocs[spaceUType].teleportSpace(entityMailbox, position, direction, context)

	def onSpaceLoseCell(self, spaceUType, spaceKey):
		"""
		define method.
		space的cell创建好了
		"""
		self._spaceAllocs[spaceUType].onSpaceGetCell(spaceKey)
		
	def onSpaceGetCell(self, spaceUType, spaceMailbox, spaceKey):
		"""
		define method.
		space的cell创建好了
		"""
		self._spaceAllocs[spaceUType].onSpaceGetCell(spaceMailbox, spaceKey)

Spaces._timermap = {}
Spaces._timermap.update(GameObject._timermap)
Spaces._timermap[wtimer.TIMER_TYPE_CREATE_SPACES] = Spaces.createSpaceOnTimer
