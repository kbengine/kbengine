# -*- coding: utf-8 -*-
import KBEngine
import GlobalConst
from KBEDebug import * 

class TAvatarInfos(dict):
	"""
	"""
	def __init__(self):
		"""
		"""
		dict.__init__(self)

	def asDict(self):
		return {
			"dbid"			: self["dbid"],
			"name"			: self["name"],
			"roleType"		: self["roleType"],
			"level"			: self["level"],
		}

	def createFromDict(self, dictData):
		self["dbid"]		= dictData["dbid"]
		self["name"]		= dictData["name"]
		self["roleType"]	= dictData["roleType"]
		self["level"]		= dictData["level"]
		return self
		
class AVATAR_INFOS_PICKLER:
	def __init__(self):
		pass

	def createObjFromDict(self, dct):
		return TAvatarInfos().createFromDict(dct)

	def getDictFromObj(self, obj):
		return obj.asDict()

	def isSameType(self, obj):
		return isinstance(obj, TAvatarInfos)

inst = AVATAR_INFOS_PICKLER()