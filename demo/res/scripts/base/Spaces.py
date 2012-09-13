# -*- coding: utf-8 -*-
import KBEngine
import Functor
from KBEDebug import *
from GameObject import GameObject
import d_spaces

class SpaceAlloc:
	def __init__(self, utype):
		self._spaces = {}
		self._utype = utype
	
	def init(self):
		"""
		"""
		spaceData = d_spaces.datas.get(self._utype)
		KBEngine.createBaseAnywhere(spaceData["entityType"], \
											{"spaceUType" : self._utype}, \
											self.onSpaceCreatedCB)

	def onSpaceCreatedCB(self, space):
		"""
		一个space创建好后的回调
		"""
		DEBUG_MSG("Spaces::create space %i. entityID=%i" % (self._utype, space.id))
		self._spaces[space.id] = space
		
	def alloc(self):
		if self._spaces == {}:
			return None
		
		return list(self._spaces.values())[0]
		
	def loginToSpace(self, avatarMailbox):
		"""
		某个玩家请求登陆到某个space中
		"""
		space = self.alloc()
		if space is None:
			ERROR_MSG("Spaces::loginToSpace: not found space %i. login to space is failed!" % self._utype)
			return
		
		space.onLoginToSpace(avatarMailbox)

	def teleportSpace(self, entityMailbox, position, direction):
		"""
		请求进入某个space中
		"""
		space = self.alloc()
		if space is None:
			ERROR_MSG("Spaces::loginToSpace: not found space %i. login to space is failed!" % self._utype)
			return
		
		DEBUG_MSG("Spaces::teleportSpace: entityMailbox=%s" % entityMailbox)
		space.teleportSpace(entityMailbox, position, direction)
		
class SpaceAllocCopy(SpaceAlloc):
	def __init__(self, utype):
		SpaceAlloc.__init__(self, utype)

		
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
			
	def loginToSpace(self, avatarMailbox, spaceUType):
		"""
		define method.
		某个玩家请求登陆到某个space中
		"""
		self._spaceAllocs[spaceUType].loginToSpace(avatarMailbox)
		
	def teleportSpace(self, entityMailbox, spaceUType, position, direction):
		"""
		define method.
		请求进入某个space中
		"""
		self._spaceAllocs[spaceUType].teleportSpace(entityMailbox, position, direction)
