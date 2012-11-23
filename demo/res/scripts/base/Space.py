# -*- coding: utf-8 -*-
import KBEngine
import random
import wtimer
from KBEDebug import *
from interfaces.GameObject import GameObject
import d_entities
import d_spaces

class Space(GameObject):
	def __init__(self):
		GameObject.__init__(self)
		self.createInNewSpace(None)
		
		self.spaceUTypeB = self.cellData["spaceUType"]
		
		# 这个地图上创建的entity总数
		self.tmpCreateEntityDatas = list(d_spaces.datas[self.spaceUTypeB].get("entities", []))
		self.avatars = {}
		
	def onLoseCell(self):
		"""
		KBEngine method.
		entity的cell部分实体丢失
		"""
		KBEngine.globalData["SpaceMgr"].onSpaceLoseCell(self.spaceUTypeB, self.spaceKey)
		GameObject.onLoseCell(self)
		
	def onGetCell(self):
		"""
		KBEngine method.
		entity的cell部分实体被创建成功
		"""
		self.addTimer(0.1, 0.1, wtimer.TIMER_TYPE_SPACE_SPAWN_TICK)
		KBEngine.globalData["SpaceMgr"].onSpaceGetCell(self.spaceUTypeB, self, self.spaceKey)
		GameObject.onGetCell(self)
		
	def spawnOnTimer(self, tid, tno):
		"""
		出生怪物
		"""
		if len(self.tmpCreateEntityDatas) <= 0:
			self.delTimer(tid)
			return
			
		entityNO = self.tmpCreateEntityDatas.pop(0)
		datas = d_entities.datas.get(entityNO)
		
		if datas is None:
			ERROR_MSG("Space::onTimer: spawn %i is error!" % entityNO)

		KBEngine.createBaseAnywhere("SpawnPoint", 
									{"spawnEntityNO"	: entityNO, 	\
									"position"			: tuple(datas.get('spawnPos', (0,0,0))), 	\
									"direction"			: (0, 0, datas.get("spawnYaw", 0)),	\
									"createToCell"		: self.cell})
				
	def loginToSpace(self, avatarMailbox):
		"""
		define method.
		某个玩家请求登陆到这个space中
		"""
		avatarMailbox.createCell(self.cell)
		self.onEnter(avatarMailbox)
		
	def logoutSpace(self, entityID):
		"""
		define method.
		某个玩家请求登出这个space
		"""
		self.onLeave(entityID)
		
	def teleportSpace(self, entityMailbox, position, direction, context):
		"""
		define method.
		请求进入某个space中
		"""
		entityMailbox.cell.onTeleportSpaceCB(self.cell, self.spaceUTypeB, position, direction)

	def onEnter(self, entityMailbox):
		"""
		defined method.
		进入场景
		"""
		self.avatars[entityMailbox.id] = entityMailbox
		self.cell.onEnter(entityMailbox)
		
	def onLeave(self, entityID):
		"""
		defined method.
		离开场景
		"""
		del self.avatars[entityID]
		self.cell.onLeave(entityID)
		
Space._timermap = {}
Space._timermap.update(GameObject._timermap)
Space._timermap[wtimer.TIMER_TYPE_SPACE_SPAWN_TICK] = Space.spawnOnTimer
