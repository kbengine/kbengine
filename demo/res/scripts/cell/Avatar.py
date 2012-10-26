# -*- coding: utf-8 -*-
import KBEngine
from KBEDebug import *
from interfaces.GameObject import GameObject
from interfaces.Combat import Combat
from interfaces.Spell import Spell
from interfaces.Teleport import Teleport
from interfaces.Dialog import Dialog
from interfaces.State import State
from interfaces.Flags import Flags

class Avatar(GameObject, 
			Flags,
			State,
			Combat, 
			Spell, 
			Teleport,
			Dialog):
	def __init__(self):
		GameObject.__init__(self) 
		Flags.__init__(self) 
		State.__init__(self) 
		Combat.__init__(self) 
		Spell.__init__(self) 
		Teleport.__init__(self) 
		Dialog.__init__(self) 
		
	def onGetWitness(self):
		"""
		KBEngine method.
		绑定了一个观察者(客户端)
		"""
		DEBUG_MSG("Avatar::onGetWitness: %i." % self.id)

	def onLoseWitness(self):
		"""
		KBEngine method.
		解绑定了一个观察者(客户端)
		"""
		DEBUG_MSG("Avatar::onLoseWitness: %i." % self.id)
	
	def onDestroy(self):
		"""
		KBEngine method.
		entity销毁
		"""
		DEBUG_MSG("Avatar::onDestroy: %i." % self.id)
		Teleport.onDestroy(self)
		
	def queryCurrSpaceRes(self, srcEntityID):
		"""
		exposed.
		查询当前场景引用的资源
		"""
		if srcEntityID != self.id:
			return
		
		DEBUG_MSG("Avatar::queryCurrSpaceRes(%i):srcEntityID=%i." % (self.id, srcEntityID))
		self.client.onQuerySpaceResResult("pic/xxx.png;pic/xxx1.png;")
		
	def queryCurrSpaceEntitys(self, srcEntityID, count):
		"""
		exposed.
		查询当前场景上的entity信息
		"""
		if srcEntityID != self.id:
			return

		
Avatar._timermap = {}
Avatar._timermap.update(GameObject._timermap)
Avatar._timermap.update(Flags._timermap)
Avatar._timermap.update(State._timermap)
Avatar._timermap.update(Combat._timermap)
Avatar._timermap.update(Spell._timermap)
Avatar._timermap.update(Teleport._timermap)
Avatar._timermap.update(Dialog._timermap)
