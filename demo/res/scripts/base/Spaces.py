# -*- coding: utf-8 -*-
import KBEngine
import Functor
from KBEDebug import *
from GameObject import GameObject
from SpaceAlloc import *
import d_spaces
		
class Spaces(GameObject):
	def __init__(self):
		GameObject.__init__(self)
		self._spaceAllocs = {}
		self.addTimer(3, 1, 1)
		self.initAlloc()
		KBEngine.globalData["SpaceMgr"] = self
	
	def initAlloc(self):
		self._tmpDatas = list(d_spaces.datas.keys())
		for utype in self._tmpDatas:
			spaceData = d_spaces.datas.get(utype)
			if spaceData["entityType"] == "SpaceCopy":
				self._spaceAllocs[utype] = SpaceAllocCopy(utype)
			else:
				self._spaceAllocs[utype] = SpaceAlloc(utype)
				
	def onTimer(self, id, userArg):
		"""
		KBEngine method.
		使用addTimer后， 当时间到达则该接口被调用
		@param id		: addTimer 的返回值ID
		@param userArg	: addTimer 最后一个参数所给入的数据
		"""
		if userArg == 1:
			if len(self._tmpDatas) > 0:
				spaceUType = self._tmpDatas.pop(0)
				self._spaceAllocs[spaceUType].init()
				
			if len(self._tmpDatas) <= 0:
				del self._tmpDatas
				self.delTimer(id)
			
	def loginToSpace(self, avatarEntity, spaceUType):
		"""
		define method.
		某个玩家请求登陆到某个space中
		"""
		self._spaceAllocs[spaceUType].loginToSpace(avatarEntity)
		
	def teleportSpace(self, entityMailbox, spaceUType, position, direction, params):
		"""
		define method.
		请求进入某个space中
		"""
		self._spaceAllocs[spaceUType].teleportSpace(entityMailbox, position, direction, params)

	def onSpaceGetCell(self, spaceUType, spaceMailbox, spaceKey):
		"""
		define method.
		space的cell创建好了
		"""
		self._spaceAllocs[spaceUType].onSpaceGetCell(spaceMailbox, spaceKey)

