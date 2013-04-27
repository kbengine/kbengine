# -*- coding: utf-8 -*-
import KBEngine
import wtimer
from KBEDebug import *
from interfaces.GameObject import GameObject
import d_entities

class SpawnPoint(GameObject):
	def __init__(self):
		KBEngine.Entity.__init__(self)
		self.addTimer(1, 0, wtimer.TIMER_TYPE_SPAWN)

	def onRestore(self):
		"""
		KBEngine method.
		entity的cell部分实体被恢复成功
		"""
		GameObject.onRestore(self)
		self.addTimer(1, 0, wtimer.TIMER_TYPE_SPAWN)
		
	def onDestroy(self):
		"""
		KBEngine method.
		当前entity马上将要被引擎销毁
		可以在此做一些销毁前的工作
		"""
		DEBUG_MSG("onDestroy(%i)" % self.id)
	
	def spawnTimer(self, tid, tno):
		datas = d_entities.datas.get(self.spawnEntityNO)
		
		if datas is None:
			ERROR_MSG("SpawnPoint::spawn:%i not found." % self.spawnEntityNO)
			return
			
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
		
SpawnPoint._timermap = {}
SpawnPoint._timermap.update(GameObject._timermap)
SpawnPoint._timermap[wtimer.TIMER_TYPE_SPAWN] = SpawnPoint.spawnTimer