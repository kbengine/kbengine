# -*- coding: utf-8 -*-
import random
import math
import time
import KBEngine
import SCDefine
from KBEDebug import *
from interfaces.Combat import Combat
from interfaces.Spell import Spell
from interfaces.Motion import Motion
from interfaces.State import State
from interfaces.Flags import Flags
from interfaces.AI import AI
from interfaces.NPCObject import NPCObject

class Monster(KBEngine.Entity,
			NPCObject, 
			Flags,
			State,
			Motion, 
			Combat, 
			Spell, 
			AI):
	def __init__(self):
		KBEngine.Entity.__init__(self)
		NPCObject.__init__(self)
		Flags.__init__(self) 
		State.__init__(self) 
		Motion.__init__(self) 
		Combat.__init__(self) 
		Spell.__init__(self) 
		AI.__init__(self) 
		
		# entity所在的层，可以设置多个不同的navmesh层来寻路, 这里20002001是warring-demo中在天上的飞龙，
		# 第0层是地面，第1层是忽略建筑物的寻路层
		if self.modelID == 20002001:
			self.layer = 1 
			
	def initEntity(self):
		"""
		virtual method.
		"""
		pass

	def checkInTerritory(self):
		"""
		virtual method.
		检查自己是否在可活动领地中
		"""
		return AI.checkInTerritory(self)

	def isMonster(self):
		"""
		virtual method.
		"""
		return True
		
	#--------------------------------------------------------------------------------------------
	#                              Callbacks
	#--------------------------------------------------------------------------------------------
	def onTimer(self, tid, userArg):
		"""
		KBEngine method.
		引擎回调timer触发
		"""
		#DEBUG_MSG("%s::onTimer: %i, tid:%i, arg:%i" % (self.getScriptName(), self.id, tid, userArg))
		NPCObject.onTimer(self, tid, userArg)
		Spell.onTimer(self, tid, userArg)
		AI.onTimer(self, tid, userArg)
		
	def onWitnessed(self, isWitnessed):
		"""
		KBEngine method.
		此实体是否被观察者(player)观察到, 此接口主要是提供给服务器做一些性能方面的优化工作，
		在通常情况下，一些entity不被任何客户端所观察到的时候， 他们不需要做任何工作， 利用此接口
		可以在适当的时候激活或者停止这个entity的任意行为。
		@param isWitnessed	: 为false时， entity脱离了任何观察者的观察
		"""
		AI.onWitnessed(self, isWitnessed)
		
	def onForbidChanged_(self, forbid, isInc):
		"""
		virtual method.
		entity禁止 条件改变
		@param isInc		:	是否是增加
		"""
		State.onForbidChanged_(self, forbid, isInc)
		AI.onForbidChanged_(self, forbid, isInc)
		
	def onStateChanged_(self, oldstate, newstate):
		"""
		virtual method.
		entity状态改变了
		"""
		State.onStateChanged_(self, oldstate, newstate)
		AI.onStateChanged_(self, oldstate, newstate)
		NPCObject.onStateChanged_(self, oldstate, newstate)
		
	def onSubStateChanged_(self, oldSubState, newSubState):
		"""
		virtual method.
		子状态改变了
		"""
		State.onSubStateChanged_(self, oldSubState, newSubState)
		AI.onSubStateChanged_(self, oldSubState, newSubState)

	def onFlagsChanged_(self, flags, isInc):
		"""
		virtual method.
		"""
		Flags.onFlagsChanged_(self, flags, isInc)
		AI.onFlagsChanged_(self, flags, isInc)

	def onEnterTrap(self, entity, range_xz, range_y, controllerID, userarg):
		"""
		KBEngine method.
		引擎回调进入陷阱触发
		"""
		AI.onEnterTrap(self, entity, range_xz, range_y, controllerID, userarg)

	def onLeaveTrap(self, entity, range_xz, range_y, controllerID, userarg):
		"""
		KBEngine method.
		引擎回调离开陷阱触发
		"""
		AI.onLeaveTrap(self, entity, range_xz, range_y, controllerID, userarg)

	def onAddEnemy(self, entityID):
		"""
		virtual method.
		有敌人进入列表
		"""
		AI.onAddEnemy(self, entityID)
		Combat.onAddEnemy(self, entityID)

	def onRemoveEnemy(self, entityID):
		"""
		virtual method.
		删除敌人
		"""
		AI.onRemoveEnemy(self, entityID)
		Combat.onRemoveEnemy(self, entityID)

	def onEnemyEmpty(self):
		"""
		virtual method.
		敌人列表空了
		"""
		AI.onEnemyEmpty(self)
		Combat.onEnemyEmpty(self)

	def onDestroy(self):
		"""
		entity销毁
		"""
		NPCObject.onDestroy(self)
		Combat.onDestroy(self)

