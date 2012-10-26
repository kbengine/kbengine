# -*- coding: utf-8 -*-
import KBEngine
import random
from KBEDebug import *
from GameObject import GameObject
import d_entities
import d_spaces

class Space(GameObject):
	def __init__(self):
		GameObject.__init__(self)
		self.createInNewSpace(None)
		
		self.spaceUTypeB = self.cellData["spaceUType"]
		
		# 这个地图上创建的entity总数
		self.tmpCreateEntityDatas = list(d_spaces.datas[self.spaceUTypeB].get("entities", []))
		
	def onGetCell(self):
		"""
		KBEngine method.
		entity的cell部分实体被创建成功
		"""
		self.addTimer(0.1, 0.1, 1)
		KBEngine.globalData["SpaceMgr"].onSpaceGetCell(self.spaceUTypeB, self, self.spaceKey)
		
	def onTimer(self, id, userArg):
		"""
		KBEngine method.
		使用addTimer后， 当时间到达则该接口被调用
		@param id		: addTimer 的返回值ID
		@param userArg	: addTimer 最后一个参数所给入的数据
		"""
		if userArg == 1:
			if len(self.tmpCreateEntityDatas) <= 0:
				self.delTimer(id)
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
		
	def teleportSpace(self, entityMailbox, position, direction, context):
		"""
		define method.
		请求进入某个space中
		"""
		entityMailbox.cell.onTeleportSpaceCB(self.cell, self.spaceUTypeB, position, direction)