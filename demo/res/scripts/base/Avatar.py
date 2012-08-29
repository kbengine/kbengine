# -*- coding: utf-8 -*-
import KBEngine
import random
from KBEDebug import *

class Avatar(KBEngine.Proxy):
	def __init__(self):
		KBEngine.Proxy.__init__(self)
		self.accountEntity = None
		self.nameB = self.cellData["name"]
		self.spaceUTypeB = self.cellData["spaceUType"]
		
	def onEntitiesEnabled(self):
		"""
		KBEngine method.
		该entity被正式激活为可使用， 此时entity已经建立了client对应实体， 可以在此创建它的
		cell部分。
		"""
		INFO_MSG("Avatar[%i-%s] entities enable. mailbox:%s" % (self.id, self.nameB, self.client))
		
		self.cellData["position"] = (random.randint(-10, 10), 250, random.randint(-10, 10))
		KBEngine.globalData["SpaceMgr"].loginToSpace(self, self.spaceUTypeB)
		
	def onGetCell(self):
		"""
		KBEngine method.
		entity的cell部分实体被创建成功
		"""
		DEBUG_MSG('Avatar::onGetCell: %s' % self.cell)
		
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
			self.accountEntity = None
			
		# 销毁base
		self.destroy()
			
	def onClientDeath(self):
		"""
		KBEngine method.
		entity丢失了客户端实体
		"""
		DEBUG_MSG("Avatar[%i].onClientDeath:" % self.id)
		# 防止正在请求创建cell的同时客户端断开了， 我们延时一秒来执行销毁cell直到销毁base
		self.addTimer(1, 0, 1)

	def onClientGetCell(self):
		"""
		KBEngine method.
		客户端已经获得了cell部分实体的相关数据
		"""
		INFO_MSG("Avatar[%i].onClientGetCell:%s" % (self.id, self.client))
		
	def onTimer(self, tid, userArg):
		DEBUG_MSG("Avatar::onTimer: %i, tid:%i, arg:%i" % (self.id, tid, userArg))
		
		if self.client is None:
			# 销毁cell实体
			self.destroyCellEntity()