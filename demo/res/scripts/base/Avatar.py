# -*- coding: utf-8 -*-
import KBEngine
import random
import wtimer
import time
import d_spaces
import d_avatar_inittab
from KBEDebug import *
from interfaces.GameObject import GameObject
from interfaces.Teleport import Teleport

class Avatar(KBEngine.Proxy,
			GameObject,
			Teleport):
	def __init__(self):
		KBEngine.Proxy.__init__(self)
		GameObject.__init__(self)
		Teleport.__init__(self)

		# 如果登录是一个副本, 无论如何登录都放置在主场景上
		spacedatas = d_spaces.datas [self.cellData["spaceUType"]]
		avatar_inittab = d_avatar_inittab.datas[self.roleType]

		if "Duplicate" in spacedatas["entityType"]:
			self.cellData["spaceUType"] = avatar_inittab["spaceUType"]
			self.cellData["direction"] = (0, 0, avatar_inittab["spawnYaw"])
			self.cellData["position"] = avatar_inittab["spawnPos"]

		self.accountEntity = None
		self.cellData["dbid"] = self.databaseID
		self.nameB = self.cellData["name"]
		self.spaceUTypeB = self.cellData["spaceUType"]
		
		self._destroyTimer = 0

	def onEntitiesEnabled(self):
		"""
		KBEngine method.
		该entity被正式激活为可使用， 此时entity已经建立了client对应实体， 可以在此创建它的
		cell部分。
		"""
		INFO_MSG("Avatar[%i-%s] entities enable. spaceUTypeB=%s, mailbox:%s" % (self.id, self.nameB, self.spaceUTypeB, self.client))
		KBEngine.globalData["SpaceMgr"].loginToSpace(self, self.spaceUTypeB, {})
		
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
	
	def destroySelf(self):
		"""
		"""
		if self.client is not None:
			return
			
		if self.cell is not None:
			# 销毁cell实体
			self.destroyCellEntity()
			return
			
		# 如果帐号ENTITY存在 则也通知销毁它
		if self.accountEntity != None:
			if time.time() - self.accountEntity.relogin > 1:
				self.accountEntity.activeCharacter = None
				self.accountEntity.destroy()
				self.accountEntity = None
			else:
				DEBUG_MSG("Avatar[%i].destroySelf: relogin =%i" % (self.id, time.time() - self.accountEntity.relogin))
				
		# 销毁base
		self.destroy()

	def onClientDeath(self):
		"""
		KBEngine method.
		entity丢失了客户端实体
		"""
		DEBUG_MSG("Avatar[%i].onClientDeath:" % self.id)
		# 防止正在请求创建cell的同时客户端断开了， 我们延时一段时间来执行销毁cell直到销毁base
		# 这段时间内客户端短连接登录则会激活entity
		self._destroyTimer = self.addTimer(1, 0, wtimer.TIMER_TYPE_DESTROY)
		
		if self.spaceID > 0:
			self.getCurrSpaceBase().logoutSpace(self.id)
		else:
			self.getSpaceMgr().logoutSpace(self.id, self.spaceID)
			
	def onClientGetCell(self):
		"""
		KBEngine method.
		客户端已经获得了cell部分实体的相关数据
		"""
		INFO_MSG("Avatar[%i].onClientGetCell:%s" % (self.id, self.client))
		
	def onDestroyTimer(self, tid, tno):
		DEBUG_MSG("Avatar::onTimer: %i, tid:%i, arg:%i" % (self.id, tid, tno))
		self.destroySelf()

Avatar._timermap = {}
Avatar._timermap.update(GameObject._timermap)
Avatar._timermap.update(Teleport._timermap)
Avatar._timermap[wtimer.TIMER_TYPE_DESTROY] = Avatar.onDestroyTimer

