# -*- coding: utf-8 -*-
import KBEngine
import random
from KBEDebug import *

class Avatar(KBEngine.Proxy):
	def __init__(self):
		KBEngine.Proxy.__init__(self)
		self.accountEntity = None
		self.cellData["name"] = "avatar%i" % self.id
		self.cellData["modelNumber"] = "ninja"
		self.cellData["modelScale"] = 0.22
		self.cellData["modelYOffset"] = 0.5
		
	def onEntitiesEnabled(self):
		"""
		KBEngine method.
		该entity被正式激活为可使用， 此时entity已经建立了client对应实体， 可以在此创建它的
		cell部分。
		"""
		INFO_MSG("Avatar[%i] entities enable. mailbox:%s" % (self.id, self.client))
		
		self.cellData["position"] = (random.randint(-10, 10), 250, random.randint(-10, 10))
		KBEngine.globalData["SpaceMgr"].loginToSpace(self, "xinshoucun")
		
	def onGetCell(self):
		"""
		KBEngine method.
		entity的cell部分实体被创建成功
		"""
		DEBUG_MSG('%s' % self.cell)
		
	def createCell(self, space):
		"""
		define method.
		创建cell实体
		"""
		self.createCellEntity(space)
	
	def onLoseCell(self):
		"""
		KBEngine method.
		entity的cell部分实体丢失
		"""
		DEBUG_MSG("Avatar[%i].onLoseCell:" % self.id)
		
		# 如果帐号ENTITY存在 则也通知销毁它
		if self.accountEntity != None:
			self.accountEntity.destroy()
			
		# 销毁base
		self.destroy()
			
	def onClientDeath(self):
		"""
		KBEngine method.
		entity丢失了客户端实体
		"""
		DEBUG_MSG("Avatar[%i].onClientDeath:" % self.id)
		# 销毁cell实体
		self.destroyCellEntity()
		
	def onClientGetCell(self):
		"""
		KBEngine method.
		客户端已经获得了cell部分实体的相关数据
		"""
		INFO_MSG("Avatar[%i].onClientGetCell:%s" % (self.id, self.client))