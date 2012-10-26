# -*- coding: utf-8 -*-
import KBEngine
from KBEDebug import *
from interfaces.GameObject import GameObject
from interfaces.Combat import Combat
from interfaces.Spell import Spell
from interfaces.teleport import teleport
from interfaces.dialog import dialog

class Avatar(GameObject, 
			Combat, 
			Spell, 
			teleport,
			dialog):
	def __init__(self):
		GameObject.__init__(self) 
		Combat.__init__(self) 
		Spell.__init__(self) 
		teleport.__init__(self) 
		dialog.__init__(self) 
		
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
Avatar._timermap.update(Combat._timermap)
Avatar._timermap.update(Spell._timermap)
Avatar._timermap.update(teleport._timermap)
Avatar._timermap.update(dialog._timermap)
