# -*- coding: utf-8 -*-
import KBEngine
from KBEDebug import *

class SpawnPoint(KBEngine.Base):
	def __init__(self):
		KBEngine.Base.__init__(self)
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
		DEBUG_MSG("cell丢失了!", self.id, self.cellData)
