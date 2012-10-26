# -*- coding: utf-8 -*-
import KBEngine
from KBEDebug import *
from interfaces.GameObject import GameObject

class SpawnPoint(GameObject):
	def __init__(self):
		GameObject.__init__(self)
		self.createCellEntity(self.createToCell)
		
	def onGetCell(self):
		"""
		KBEngine method.
		entity的cell部分实体被创建成功
		"""
		DEBUG_MSG("创建cell成功!")
		
		# 销毁cell部分实体
		# self.destroyCellEntity()
		
	def onLoseCell(self):
		"""
		KBEngine method.
		entity的cell部分实体丢失
		"""
		DEBUG_MSG("cell丢失了! %s" % (self.id, self.cellData))
		self.destroy()
		
SpawnPoint._timermap = {}
SpawnPoint._timermap.update(GameObject._timermap)
