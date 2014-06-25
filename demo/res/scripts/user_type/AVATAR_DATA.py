# -*- coding: utf-8 -*-
import KBEngine
import GlobalConst
from KBEDebug import * 

class TAvatarData(dict):
	"""
	"""
	def __init__(self):
		"""
		"""
		dict.__init__(self)
		
	def asDict(self):
		for key, val in self.items():
			return {"param1" : val[0], "param2" : val[1]}

	def createFromDict(self, dictData):
		self[dictData["param1"]] = [dictData["param1"], dictData["param2"]]
		return self
		
class AVATAR_DATA_PICKLER:
	def __init__(self):
		pass

	def createObjFromDict(self, dct):
		return TAvatarData().createFromDict(dct)

	def getDictFromObj(self, obj):
		return obj.asDict()

	def isSameType(self, obj):
		return isinstance(obj, TAvatarData)

inst = AVATAR_DATA_PICKLER()