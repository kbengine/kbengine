# -*- coding: utf-8 -*-
import KBEngine
import GlobalConst
from KBEDebug import * 

class TAvatarInfos(list):
	"""
	"""
	def __init__(self):
		"""
		"""
		list.__init__(self)
		
	def asDict(self):
		data = {
			"dbid"			: self[0],
			"name"			: self[1],
			"roleType"		: self[2],
			"level"			: self[3],
			"data"			: self[4],
		}
		
		return data

	def createFromDict(self, dictData):
		self.extend([dictData["dbid"], dictData["name"], dictData["roleType"], dictData["level"], dictData["data"]])
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

avatar_info_inst = AVATAR_INFOS_PICKLER()

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
			datas.append(val)
			
		return dct

	def createFromDict(self, dictData):
		for data in dictData["values"]:
			self[data[0]] = data
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

avatar_info_list_inst = AVATAR_INFOS_LIST_PICKLER()