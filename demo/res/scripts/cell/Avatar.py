# -*- coding: utf-8 -*-
import KBEngine
import dialog
from KBEDebug import *
from GameObject import GameObject
from Combat import Combat
from Spell import Spell

class Avatar(GameObject, Combat, Spell):
	def __init__(self):
		GameObject.__init__(self) 
		Combat.__init__(self) 
		Spell.__init__(self) 
	
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
		
	def dialog(self, srcEntityID, targetID, dialogID):
		"""
		exposed.
		对一个目标entity施放一个技能
		"""
		if srcEntityID != self.id:
			return
			
		if not KBEngine.entities.has_key(targetID):
			DEBUG_MSG("Avatar::dialog: %i not found targetID:%i" % (self.id, dialogID))
			return
			
		dialog.onGossip(dialogID, self, KBEngine.entities[targetID])

Avatar._timermap = {}
Avatar._timermap.update(GameObject._timermap)
Avatar._timermap.update(Spell._timermap)