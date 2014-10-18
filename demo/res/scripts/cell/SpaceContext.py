# -*- coding: utf-8 -*-
import KBEngine
from KBEDebug import *
import d_spaces

class SpaceContext(dict):
	"""
	产生space上下文
	"""
	def __init__(self, entity):
		pass
	
	@staticmethod
	def create(entity):
		return {}
		
		
class SpaceDuplicateContext(SpaceContext):
	"""
	产生space上下文
	"""
	def __init__(self, entity):
		SpaceContext.__init__(self, entity)

	@staticmethod
	def create(entity):
		return {"spaceKey" : entity.dbid}
		
def createContext(entity, spaceUType):
	"""
	"""
	spaceData = d_spaces.datas.get(spaceUType)
	
	return {
		"Space" : SpaceContext,
		"SpaceDuplicate" : SpaceDuplicateContext,
	}[spaceData["entityType"]].create(entity)