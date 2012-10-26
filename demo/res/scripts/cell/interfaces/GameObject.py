# -*- coding: utf-8 -*-
import KBEngine
from KBEDebug import * 
import d_spaces
import wtimer

class GameObject(KBEngine.Entity):
	def __init__(self):
		KBEngine.Entity.__init__(self)

	def getScriptName(self):
		return self.__class__.__name__
		
	def onTimer(self, tid, userArg):
		#DEBUG_MSG("%s::onTimer: %i, tid:%i, arg:%i" % (self.getScriptName(), self.id, tid, userArg))
		self._timermap[userArg](self, tid, userArg)
		
	def getCurrSpaceBase(self):
		"""
		获得当前space的entity baseMailbox
		"""
		return KBEngine.globalData["space_%i" % self.spaceID]

	def getCurrSpace(self):
		"""
		获得当前space的entity baseMailbox
		"""
		spaceBase = self.getCurrSpaceBase()
		return KBEngine.entities.get(spaceBase.id, None)
		
	def getSpaceMgr(self):
		"""
		获取场景管理器
		"""
		return KBEngine.globalData["SpaceMgr"]


GameObject._timermap = {}
