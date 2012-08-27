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
				spaceID = self._tmpDatas.pop(0)
				spaceData = d_spaces.datas.get(spaceID)
				space = KBEngine.createBaseAnywhere(spaceData["entityType"], \
													{"utype" : spaceID}, \
													Functor.Functor(self.onSpaceCreatedCB, spaceID))
				
			if len(self._tmpDatas) <= 0:
				self.delTimer(id)
				
	def onSpaceCreatedCB(self, spaceID, space):
		"""
		一个space创建好后的回调
		"""
		DEBUG_MSG("Spaces::create space %s. entityID=%i" % (spaceID, space.id))
		self._spaces[spaceID] = space
		
	def loginToSpace(self, avatarMailbox, spaceName):
		"""
		define method.
		某个玩家请求登陆到某个space中
		"""
		space = self._spaces.get(spaceName)
		if spaceName not in self._spaces:
			ERROR_MSG("Spaces::not found space %s. login to space is failed!" % spaceName)
			return
		
		space.onLoginToSpace(avatarMailbox)