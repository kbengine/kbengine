# -*- coding: utf-8 -*-
import KBEngine
from KBEDebug import *
from interfaces.GameObject import GameObject
import d_entities

class SpawnPoint(GameObject):
	def __init__(self):
		KBEngine.Entity.__init__(self)
		
		datas = d_entities.datas.get(self.spawnEntityNO)
		DEBUG_MSG("spawn:%i" % self.spawnEntityNO)
		params = {
			"spawnPos" : tuple(datas["spawnPos"]),
			"uid" : datas["id"],
			"utype" : datas["etype"],
			"modelID" : datas["modelID"],
			"dialogID" : datas["dialogID"],
			"name" : datas["name"],
			"descr" : datas.get("descr", ''),
		}
		
		e = KBEngine.createEntity(datas["entityType"], self.spaceID, tuple(self.position), tuple(self.direction), params)
		
	def onDestroy(self):
		"""
		KBEngine method.
		当前entity马上将要被引擎销毁
		可以在此做一些销毁前的工作
		"""
		DEBUG_MSG("onDestroy(%i)" % self.id)
		
SpawnPoint._timermap = {}
SpawnPoint._timermap.update(GameObject._timermap)
