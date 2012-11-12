# -*- coding: utf-8 -*-
import KBEngine
import GlobalConst
from KBEDebug import * 

class TAvatarInfosList(dict):
	"""
	"""
	def __init__(self):
		"""
		"""
		dict.__init__(self)
		
	def asDict(self):
		datas = []
		dct = {"values" : datas}

		for key, val in self.items():
			data = {
				"dbid"			: key,
				"name"			: val[0],
				"roleType"		: val[1],
				"level"			: val[2],
			}
			datas.append(data)
			
		return dct

	def createFromDict(self, dictData):
		for data in dictData["values"]:
			self[data["dbid"]] = [data["name"], data["roleType"], data["level"]]
		return self
		
class AVATAR_INFOS_LIST_PICKLER:
	def __init__(self):
		pass

	def createObjFromDict(self, dct):
		return TAvatarInfosList().createFromDict(dct)

	def getDictFromObj(self, obj):
		return obj.asDict()

	def isSameType(self, obj):
		return isinstance(obj, TAvatarInfosList)

inst = AVATAR_INFOS_LIST_PICKLER()