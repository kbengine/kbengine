# -*- coding: utf-8 -*-
import KBEngine
from KBEDebug import *

class Space(KBEngine.Entity):
	def __init__(self):
		KBEngine.Entity.__init__(self)
		DEBUG_MSG('created space[%d] entityID = %i.' % (self.spaceUType, self.id))
		#self.addSpaceGeometryMapping(self.spaceID, self.spaceName)
		
		# 记录地图上所有的实体ID， 按类型区分
		self.allNPEntities = [] 
		
		KBEngine.globalData["space_%i" % self.spaceID] = self
		
	def regEntity(self, entityType, entityInfo):
		self.allNPEntities.append(entityInfo)