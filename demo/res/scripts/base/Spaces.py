# -*- coding: utf-8 -*-
import KBEngine
import Functor
from KBEDebug import *
from GameObject import GameObject

# 游戏存在的所有地图名称
SPACE_NAMES = ["mengzhong", "xinshoucun"]

class Spaces(GameObject):
	def __init__(self):
		GameObject.__init__(self)
		self._spaces = {}
		self.addTimer(15, 1, 1)
		
		KBEngine.globalData["SpaceMgr"] = self
		
	def onTimer(self, id, userArg):
		"""
		KBEngine method.
		使用addTimer后， 当时间到达则该接口被调用
		@param id		: addTimer 的返回值ID
		@param userArg	: addTimer 最后一个参数所给入的数据
		"""
		if userArg == 1:
			if len(SPACE_NAMES) > 0:
				spaceName = SPACE_NAMES.pop(0)
				space = KBEngine.createBaseAnywhere("Space", \
													{"spaceName" : spaceName}, \
													Functor.Functor(self.onSpaceCreatedCB, spaceName))
				
			if len(SPACE_NAMES) <= 0:
				self.delTimer(id)
				
	def onSpaceCreatedCB(self, spaceName, space):
		"""
		一个space创建好后的回调
		"""
		DEBUG_MSG("create space %s. entityID=%i" % (spaceName, space.id))
		self._spaces[spaceName] = space
		
	def loginToSpace(self, avatarMailbox, spaceName):
		"""
		define method.
		某个玩家请求登陆到某个space中
		"""
		space = self._spaces.get(spaceName)
		if spaceName not in self._spaces:
			ERROR_MSG("not found space %s. login to space is failed!" % spaceName)
			return
		
		space.onLoginToSpace(avatarMailbox)