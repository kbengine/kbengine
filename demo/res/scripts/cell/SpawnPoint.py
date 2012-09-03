# -*- coding: utf-8 -*-
import KBEngine
from KBEDebug import *
import d_entities

class SpawnPoint(KBEngine.Entity):
	def __init__(self):
		KBEngine.Entity.__init__(self)
		
		datas = d_entities.datas.get(self.spawnEntityNO)
		params = {
			"spawnPos" : tuple(self.position),
			"utype" : datas["id"],
		}
		
		e = KBEngine.createEntity(datas["entityType"], self.spaceID, tuple(self.position), tuple(self.direction), params)
		
		info = {
			"entityID" : e.id,
			"utype" : datas["id"],
			"spawnPos" : datas["spawnPos"],
			"modelID" : datas["modelID"],
			"dialogID" : datas["dialogID"],
		}
		KBEngine.globalData["space_%i" % self.spaceID].regEntity(datas["entityType"], info)
		
	def onDestroy(self):
		"""
		KBEngine method.
		当前entity马上将要被引擎销毁
		可以在此做一些销毁前的工作
		"""
		DEBUG_MSG("onDestroy(%i)" % self.id)
		
