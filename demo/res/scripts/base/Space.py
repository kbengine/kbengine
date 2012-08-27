# -*- coding: utf-8 -*-
import KBEngine
import random
from KBEDebug import *
from GameObject import GameObject
import d_entities

class Space(GameObject):
	def __init__(self):
		GameObject.__init__(self)
		self.createInNewSpace(None)
		# 这个地图上创建的entity总数
		self.tmpCreateEntityDatas = list(d_entities.datas.keys())

	def onGetCell(self):
		"""
		KBEngine method.
		entity的cell部分实体被创建成功
		"""
		self.addTimer(3, 1, 1)
		
	def onTimer(self, id, userArg):
		"""
		KBEngine method.
		使用addTimer后， 当时间到达则该接口被调用
		@param id		: addTimer 的返回值ID
		@param userArg	: addTimer 最后一个参数所给入的数据
		"""
		if userArg == 1:
			entityNO = self.tmpCreateEntityDatas.pop(0)
			KBEngine.createBaseAnywhere("SpawnPoint", 
										{"spawnEntityNO"	: entityNO, 	\
										"position"			: (random.randint(-512, 512), 250, random.randint(-512, 512)), 	\
										"direction"			: (0, 0, 0),	\
										"createToCell"		: self.cell})
			
			if len(self.tmpCreateEntityDatas) <= 0:
				self.delTimer(id)
				
	def onLoginToSpace(self, avatarMailbox):
		"""
		define method.
		某个玩家请求登陆到这个space中
		"""
		avatarMailbox.createCell(self.cell)