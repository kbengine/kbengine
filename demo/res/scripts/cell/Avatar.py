# -*- coding: utf-8 -*-
import KBEngine
from KBEDebug import *
from GameObject import GameObject

class Avatar(GameObject):
	def __init__(self):
		GameObject.__init__(self) 
		#self.addTimer(1, 1, 1)
		
	def onTimer(self, tid, userArg):
		DEBUG_MSG("Avatar::onTimer: %i, tid:%i, arg:%i" % (self.id, tid, userArg))
	
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
		
	def queryCurrSpaceEntitys(self, srcEntityID, count):
		"""
		exposed.
		查询当前场景上的entity信息
		"""
		DEBUG_MSG("Avatar::queryCurrSpaceEntitys(%i):srcEntityID=%i, count=%i" % (self.id, srcEntityID, count))
		space = KBEngine.globalData["space_%i" % self.spaceID]
		
		allDatas = list(space.allNPEntities)
		while len(allDatas) > 0:
			data = allDatas.pop(0)
			self.client.onQueryEntityResult(data)

		end = {
			"entityID" : 0,
			"utype" : 0,
			"spawnPos" : (0, 0, 0),
			"modelID" : 0,
			"dialogID" : 0,
		}
		self.client.onQueryEntityResult(end)
		
	def spellTarget(self, srcEntityID, skillID, targetID):
		"""
		exposed.
		对一个目标entity施放一个技能
		"""
		if srcEntityID != self.id:
			return
			
		DEBUG_MSG("Avatar::spellTarget(%i):skillID=%i, srcEntityID=%i, targetID=%i" % (self.id, skillID, srcEntityID, targetID))