# -*- coding: utf-8 -*-
import KBEngine
import d_spaces
import SCDefine
import GlobalDefine
from KBEDebug import * 

import d_entities
import d_avatar_inittab

class GameObject:
	"""
	服务端游戏对象的基础接口类
	"""
	def __init__(self):
		pass

	def initEntity(self):
		"""
		virtual method.
		"""
		pass
	
	def isPlayer(self):
		"""
		virtual method.
		"""
		return False

	def isNPC(self):
		"""
		virtual method.
		"""
		return False
		
	def isMonster(self):
		"""
		virtual method.
		"""
		return False
	
	def getDatas(self):
		if self.isPlayer():
			return d_avatar_inittab.datas[self.uid]
		
		return d_entities.datas[self.uid]
		
	def getScriptName(self):
		return self.__class__.__name__
		
	def onTimer(self, tid, userArg):
		"""
		KBEngine method.
		引擎回调timer触发
		"""
		#DEBUG_MSG("%s::onTimer: %i, tid:%i, arg:%i" % (self.getScriptName(), self.id, tid, userArg))
		self._timermap[userArg](self, tid, userArg)
		
	def getCurrSpaceBase(self):
		"""
		获得当前space的entity baseMailbox
		"""
		return KBEngine.globalData["space_%i" % self.spaceID]

	def getCurrSpace(self):
		"""
		获得当前space的entity
		"""
		spaceBase = self.getCurrSpaceBase()
		return KBEngine.entities.get(spaceBase.id, None)
		
	def getSpaces(self):
		"""
		获取场景管理器
		"""
		return KBEngine.globalData["Spaces"]
	
	def startDestroyTimer(self):
		"""
		virtual method.
		
		启动销毁entitytimer
		"""
		if self.isState(GlobalDefine.ENTITY_STATE_DEAD):
			self.addTimer(5, 0, SCDefine.TIMER_TYPE_DESTROY)
			DEBUG_MSG("%s::startDestroyTimer: %i running." % (self.getScriptName(), self.id))
			
	def onStateChanged_(self, oldstate, newstate):
		"""
		virtual method.
		entity状态改变了
		"""
		self.startDestroyTimer()
			
	def onWitnessed(self, isWitnessed):
		"""
		KBEngine method.
		此实体是否被观察者(player)观察到, 此接口主要是提供给服务器做一些性能方面的优化工作，
		在通常情况下，一些entity不被任何客户端所观察到的时候， 他们不需要做任何工作， 利用此接口
		可以在适当的时候激活或者停止这个entity的任意行为。
		@param isWitnessed	: 为false时， entity脱离了任何观察者的观察
		"""
		DEBUG_MSG("%s::onWitnessed: %i isWitnessed=%i." % (self.getScriptName(), self.id, isWitnessed))
		
	def onEnterTrap(self, entityEntering, range_xz, range_y, controllerID, userarg):
		"""
		KBEngine method.
		引擎回调进入陷阱触发
		"""
		if entityEntering.getScriptName() == "Avatar":
			DEBUG_MSG("%s::onEnterTrap: %i entityEntering=%i, range_xz=%s, range_y=%s, controllerID=%i, userarg=%i" % \
							(self.getScriptName(), self.id, entityEntering.id, range_xz, range_y, controllerID, userarg))

	def onLeaveTrap(self, entityLeaving, range_xz, range_y, controllerID, userarg):
		"""
		KBEngine method.
		引擎回调离开陷阱触发
		"""
		if entityLeaving.getScriptName() == "Avatar":
			DEBUG_MSG("%s::onLeaveTrap: %i entityLeaving=%i, range_xz=%s, range_y=%s, controllerID=%i, userarg=%i" % \
							(self.getScriptName(), self.id, entityLeaving.id, range_xz, range_y, controllerID, userarg))

	def onRestore(self):
		"""
		KBEngine method.
		entity的cell部分实体被恢复成功
		"""
		DEBUG_MSG("%s::onRestore: %s" % (self.getScriptName(), self.base))

	def onDestroyEntityTimer(self, tid, tno):
		"""
		entity的延时销毁timer
		"""
		self.destroy()
		
GameObject._timermap = {}
GameObject._timermap[SCDefine.TIMER_TYPE_DESTROY] = GameObject.onDestroyEntityTimer