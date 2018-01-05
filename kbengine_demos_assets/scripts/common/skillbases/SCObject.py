# -*- coding: utf-8 -*-
#
"""
"""

import KBEngine
import GlobalDefine

g_scobject_maps = {}

class SCObject:
	"""
	"""
	_c_type = GlobalDefine.SKILL_OBJECT_TYPE_UNKNOWN
	def __init__(self, obj):
		"""
		"""
		super(SCObject, self).__init__()
		self.realobj = obj
		
	def getObject(self):
		return self.realobj

	def getObjectReal(self):
		return self.realobj

	def getPosition(self):
		return (0,0,0)

	def getDirection(self):
		return (0,0,0)

	def getReference(self, entity):
		return entity

	def distToDelay(self, flySpeed, position):
		return 0.0

	def getID(self):
		return 0

	@classmethod
	def getType(SELF):
		return SELF._c_type

	@classmethod
	def isType(SELF, otype):
		return SELF._c_type == otype

	def asDict(self):
		return {
			"type"	: self.getType(),
			"param"	: self.realobj,
		}

class SCEntity(SCObject):
	"""
	"""
	_c_type = GlobalDefine.SKILL_OBJECT_TYPE_ENTITY
	def __init__(self, obj):
		"""
		"""
		super(SCEntity, self).__init__(obj)

	def getObject(self):
		return KBEngine.entities.get(self.realobj)

	def getPosition(self):
		entity = self.getObject()
		if not entity:
			return (0,0,0)
		return entity.position

	def getDirection(self):
		entity = self.getObject()
		if not entity:
			return (0,0,0)
		return entity.direction

	def getReference(self, entity):
		return self.getObject()

	def getID(self):
		return self.realobj

	def distToDelay(self, flySpeed, position):
		"""
		至少1m/s，小于1米/秒则当作是瞬发处理
		"""
		entity = self.getObject()
		if entity:
			if flySpeed > 1.0:
				return position.distTo(entity.position) / flySpeed
		return 0.0

class SCPosition(SCObject):
	"""
	"""
	_c_type = GlobalDefine.SKILL_OBJECT_TYPE_POSITION
	def __init__(self, obj):
		"""
		"""
		super(SCPosition, self).__init__(obj)

	def getPosition(self):
		return self.realobj

	def getDirection(self):
		return (0,0,0)

	def getReference(self, entity):
		return entity

	def distToDelay(self, flySpeed, position):
		"""
		至少1m/s，小于1米/秒则当作是瞬发处理
		"""
		if flySpeed > 1.0:
			return position.flatDistTo(self.realobj) / flySpeed
		return 0.0

g_scobject_maps[GlobalDefine.SKILL_OBJECT_TYPE_ENTITY] = SCEntity
g_scobject_maps[GlobalDefine.SKILL_OBJECT_TYPE_POSITION] = SCPosition

def createSCEntity(entity):
	return SCEntity(entity.id)

def createSCEntityByID(entityID):
	return SCEntity(entityID)

def createSCPosition(position):
		return SCPosition(position)

def createSCObject(type, obj):
	return g_scobject_maps[type](obj)
