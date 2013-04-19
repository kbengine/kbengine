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
		"""
		KBEngine method.
		引擎回调timer触发
		"""
		if self.isDestroyed:
			self.delTimer(tid)
			return
			
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

	def onWitnessed(self, isWitnessed):
		"""
		KBEngine method.
		此实体是否被观察者(player)观察到, 此接口主要是提供给服务器做一些性能方面的优化工作，
		在通常情况下，一些entity不被任何客户端所观察到的时候， 他们不需要做任何工作， 利用此接口
		可以在适当的时候激活或者停止这个entity的任意行为。
		@param isWitnessed	: 为false时， entity脱离了任何观察者的观察
		"""
		DEBUG_MSG("%s::onWitnessed: %i isWitnessed=%i." % (self.getScriptName(), self.id, isWitnessed))
		
	def onEnterTrap(self, entity, range_xz, range_y, controllerID, userarg):
		"""
		KBEngine method.
		引擎回调进入陷阱触发
		"""
		if entity.getScriptName() == "Avatar":
			DEBUG_MSG("%s::onEnterTrap: %i entity=%i, range_xz=%s, range_y=%s, controllerID=%i, userarg=%i" % \
							(self.getScriptName(), self.id, entity.id, range_xz, range_y, controllerID, userarg))

	def onLeaveTrap(self, entity, range_xz, range_y, controllerID, userarg):
		"""
		KBEngine method.
		引擎回调离开陷阱触发
		"""
		if entity.getScriptName() == "Avatar":
			DEBUG_MSG("%s::onLeaveTrap: %i entity=%i, range_xz=%s, range_y=%s, controllerID=%i, userarg=%i" % \
							(self.getScriptName(), self.id, entity.id, range_xz, range_y, controllerID, userarg))

						
GameObject._timermap = {}
