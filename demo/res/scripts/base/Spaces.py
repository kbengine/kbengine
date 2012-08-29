# -*- coding: utf-8 -*-
import KBEngine
import Functor
from KBEDebug import *
from GameObject import GameObject
import d_spaces

class Spaces(GameObject):
	def __init__(self):
		GameObject.__init__(self)
		self._spaces = {}
		self.addTimer(3, 1, 1)
		self._tmpDatas = list(d_spaces.datas.keys())
		
		KBEngine.globalData["SpaceMgr"] = self
		
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
				spaceData = d_spaces.datas.get(spaceUType)
				space = KBEngine.createBaseAnywhere(spaceData["entityType"], \
													{"spaceUType" : spaceUType}, \
													Functor.Functor(self.onSpaceCreatedCB, spaceUType))
				
			if len(self._tmpDatas) <= 0:
				self.delTimer(id)
				
	def onSpaceCreatedCB(self, spaceUType, space):
		"""
		一个space创建好后的回调
		"""
		DEBUG_MSG("Spaces::create space %i. entityID=%i" % (spaceUType, space.id))
		self._spaces[spaceUType] = space
		
	def loginToSpace(self, avatarMailbox, spaceUType):
		"""
		define method.
		某个玩家请求登陆到某个space中
		"""
		space = self._spaces.get(spaceUType)
		if space is None:
			ERROR_MSG("Spaces::loginToSpace: not found space %i. login to space is failed!" % spaceUType)
			return
		
		space.onLoginToSpace(avatarMailbox)
		
	def teleportTo(self, entityMailbox, spaceUType):
		"""
		define method.
		请求进入某个space中
		"""
		space = self._spaces.get(spaceUType)
		if space is None:
			ERROR_MSG("Spaces::teleportTo: not found space %i. teleportTo is failed!" % spaceUType)
			return
		
		space.teleportTo(avatarMailbox)