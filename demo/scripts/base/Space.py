# -*- coding: utf-8 -*-
import KBEngine
import random
import wtimer
import copy
import math
from KBEDebug import *
from interfaces.GameObject import GameObject
import d_entities
import d_spaces
import d_spaces_spawns
import xml.etree.ElementTree as etree 

class Space(KBEngine.Base, GameObject):
	def __init__(self):
		KBEngine.Base.__init__(self)
		GameObject.__init__(self)
		self.createInNewSpace(None)
		
		self.spaceUTypeB = self.cellData["spaceUType"]
		
		self.spaceResName = d_spaces.datas.get(self.spaceUTypeB)['resPath']
		
		# 这个地图上创建的entity总数
		self.tmpCreateEntityDatas = copy.deepcopy(d_spaces_spawns.datas.get(self.spaceUTypeB, ()))
		
		self.avatars = {}
		self.createSpawnPointDatas()
		
	def createSpawnPointDatas(self):
		"""
		"""
		res = r"scripts\data\spawnpoints\%s_spawnpoints.xml" % self.spaceResName
		if(len(self.spaceResName) == 0 or not KBEngine.hasRes(res)):
			return
			
		res = KBEngine.getResFullPath(res)
			
		tree = etree.parse(res) 
		root = tree.getroot()
		
		DEBUG_MSG("Space::createSpawnPointDatas: %s" % (res))
		
		for child in root:
			position = child[0][0]
			direction = child[0][1]
			scaleNode = child[0][2]
			scale = int(((float(scaleNode[0].text) + float(scaleNode[1].text) + float(scaleNode[2].text)) / 3.0) * 10)
			self.tmpCreateEntityDatas.append([int(child.attrib['name']), \
			(float(position[0].text), float(position[1].text), float(position[2].text)), \
			(float(direction[0].text) * ((math.pi * 2) / 360), float(direction[1].text) * ((math.pi * 2) / 360), float(direction[2].text) * ((math.pi * 2) / 360)), \
			scale, \
			])
		
	def onLoseCell(self):
		"""
		KBEngine method.
		entity的cell部分实体丢失
		"""
		KBEngine.globalData["Spaces"].onSpaceLoseCell(self.spaceUTypeB, self.spaceKey)
		GameObject.onLoseCell(self)
		
	def onGetCell(self):
		"""
		KBEngine method.
		entity的cell部分实体被创建成功
		"""
		DEBUG_MSG("Space::onGetCell: %i" % self.id)
		self.addTimer(0.1, 0.1, wtimer.TIMER_TYPE_SPACE_SPAWN_TICK)
		KBEngine.globalData["Spaces"].onSpaceGetCell(self.spaceUTypeB, self, self.spaceKey)
		GameObject.onGetCell(self)
		
	def spawnOnTimer(self, tid, tno):
		"""
		出生怪物
		"""
		if len(self.tmpCreateEntityDatas) <= 0:
			self.delTimer(tid)
			return
			
		datas = self.tmpCreateEntityDatas.pop(0)
		
		if datas is None:
			ERROR_MSG("Space::onTimer: spawn %i is error!" % datas[0])

		KBEngine.createBaseAnywhere("SpawnPoint", 
									{"spawnEntityNO"	: datas[0], 	\
									"position"			: datas[1], 	\
									"direction"			: datas[2],		\
									"modelScale"		: datas[3],		\
									"createToCell"		: self.cell})
				
	def loginToSpace(self, avatarMailbox, context):
		"""
		defined method.
		某个玩家请求登陆到这个space中
		"""
		avatarMailbox.createCell(self.cell)
		self.onEnter(avatarMailbox)
		
	def logoutSpace(self, entityID):
		"""
		defined method.
		某个玩家请求登出这个space
		"""
		self.onLeave(entityID)
		
	def teleportSpace(self, entityMailbox, position, direction, context):
		"""
		defined method.
		请求进入某个space中
		"""
		entityMailbox.cell.onTeleportSpaceCB(self.cell, self.spaceUTypeB, position, direction)

	def onEnter(self, entityMailbox):
		"""
		defined method.
		进入场景
		"""
		self.avatars[entityMailbox.id] = entityMailbox
		
		if self.cell is not None:
			self.cell.onEnter(entityMailbox)
		
	def onLeave(self, entityID):
		"""
		defined method.
		离开场景
		"""
		if entityID in self.avatars:
			del self.avatars[entityID]
		
		if self.cell is not None:
			self.cell.onLeave(entityID)
		
Space._timermap = {}
Space._timermap.update(GameObject._timermap)
Space._timermap[wtimer.TIMER_TYPE_SPACE_SPAWN_TICK] = Space.spawnOnTimer
