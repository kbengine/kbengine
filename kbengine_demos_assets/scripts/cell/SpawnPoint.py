# -*- coding: utf-8 -*-
import KBEngine
import SCDefine
from KBEDebug import *
from interfaces.GameObject import GameObject
import d_entities

class SpawnPoint(KBEngine.Entity, GameObject):
	def __init__(self):
		KBEngine.Entity.__init__(self)
		self.addTimer(1, 0, SCDefine.TIMER_TYPE_SPAWN)
		
	def spawnTimer(self):
		datas = d_entities.datas.get(self.spawnEntityNO)
		
		if datas is None:
			ERROR_MSG("SpawnPoint::spawn:%i not found." % self.spawnEntityNO)
			return
			
		params = {
			"spawnID"	: self.id,
			"spawnPos" : tuple(self.position),
			"uid" : datas["id"],
			"utype" : datas["etype"],
			"modelID" : datas["modelID"],
			"modelScale" : self.modelScale,
			"dialogID" : datas["dialogID"],
			"name" : datas["name"],
			"descr" : datas.get("descr", ''),
		}
		
		e = KBEngine.createEntity(datas["entityType"], self.spaceID, tuple(self.position), tuple(self.direction), params)

	#--------------------------------------------------------------------------------------------
	#                              Callbacks
	#--------------------------------------------------------------------------------------------
	def onTimer(self, tid, userArg):
		"""
		KBEngine method.
		引擎回调timer触发
		"""
		#DEBUG_MSG("%s::onTimer: %i, tid:%i, arg:%i" % (self.getScriptName(), self.id, tid, userArg))
		if SCDefine.TIMER_TYPE_SPAWN == userArg:
			self.spawnTimer()
		
		GameObject.onTimer(self, tid, userArg)

	def onRestore(self):
		"""
		KBEngine method.
		entity的cell部分实体被恢复成功
		"""
		GameObject.onRestore(self)
		self.addTimer(1, 0, SCDefine.TIMER_TYPE_SPAWN)
		
	def onDestroy(self):
		"""
		KBEngine method.
		当前entity马上将要被引擎销毁
		可以在此做一些销毁前的工作
		"""
		DEBUG_MSG("onDestroy(%i)" % self.id)
	
	def onEntityDestroyed(self, entityNO):
		"""
		defined.
		出生的entity销毁了 需要重建?
		"""
		self.addTimer(1, 0, SCDefine.TIMER_TYPE_SPAWN)
		
