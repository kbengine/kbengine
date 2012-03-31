# -*- coding: utf-8 -*-
import KBEngine
from KBEDebug import *

class SpawnPoint(KBEngine.Entity):
	def __init__(self):
		KBEngine.Entity.__init__(self)
		
		params = {
			"spawnPos" : tuple(self.position),
		}
		
		KBEngine.createEntity(self.spawnEntityType, self.spaceID, tuple(self.position), tuple(self.direction), params)
		
	def onDestroy(self):
		"""
		KBEngine method.
		当前entity马上将要被引擎销毁
		可以在此做一些销毁前的工作
		"""
		DEBUG_MSG("我要销毁了", self.id)