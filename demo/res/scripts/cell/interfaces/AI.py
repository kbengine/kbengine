# -*- coding: utf-8 -*-
import KBEngine
import wtimer
import time
import random
import GlobalDefine
from KBEDebug import * 

__TERRITORY_AREA__ = 30.0

class AI:
	def __init__(self):
		pass
	
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
		ret = self.position.distTo(self.spawnPos) <= __TERRITORY_AREA__
		if not ret:
			INFO_MSG("%s::checkInTerritory: %i is False." % (self.getScriptName(), self.id))
			
		return ret

	def addTerritory(self):
		"""
		添加领地
		进入领地范围的某些entity将视为敌人
		"""
		assert self.territoryControllerID == 0 and "territoryControllerID != 0"
		trange = __TERRITORY_AREA__ / 2.0
		self.territoryControllerID = self.addProximity(trange, 0, 0)
		
		if self.territoryControllerID <= 0:
			ERROR_MSG("%s::addTerritory: %i, range=%i, is error!" % (self.getScriptName(), self.id, trange))
		else:
			INFO_MSG("%s::addTerritory: %i range=%i, id=%i." % (self.getScriptName(), self.id, trange, self.territoryControllerID))
			
	def delTerritory(self):
		"""
		删除领地
		"""
		if self.territoryControllerID > 0:
			self.cancel(self.territoryControllerID)
			self.territoryControllerID = 0
			INFO_MSG("%s::delTerritory: %i" % (self.getScriptName(), self.id))
			
	def enable(self):
		"""
		激活entity
		"""
		self.heartBeatTimerID = \
		self.addTimer(random.randint(0, 1), 1, wtimer.TIMER_TYPE_HEARDBEAT)				# 心跳timer, 每1秒一次
		
	def disable(self):
		"""
		禁止这个entity做任何行为
		"""
		self.delTimer(self.heartBeatTimerID)
		self.heartBeatTimerID = 0
	
	def think(self):
		"""
		virtual method.
		"""
		if self.isState(GlobalDefine.ENTITY_STATE_FREE):
			self.onThinkFree()
		elif self.isState(GlobalDefine.ENTITY_STATE_FIGHT):
			self.onThinkFight()
		else:
			self.onThinkOther()
		
		if not self.isWitnessed:
			self.disable()
		
	def choiceTarget(self):
		"""
		从仇恨表选择一个敌人
		"""
		if len(self.enemyLog) > 0:
			self.targetID = self.enemyLog[0]
		else:
			self.targetID = 0
	
	def setTarget(self, entityID):
		"""
		设置目标
		"""
		self.targetID = entityID
		self.onTargetChanged()
	
	# ----------------------------------------------------------------
	# callback
	# ----------------------------------------------------------------
	def onHeardTimer(self, tid, tno):
		"""
		entity的心跳
		"""
		self.think()
		
	def onTargetChanged(self):
		"""
		virtual method.
		目标改变
		"""
		pass
		
	def onWitnessed(self, isWitnessed):
		"""
		KBEngine method.
		此实体是否被观察者(player)观察到, 此接口主要是提供给服务器做一些性能方面的优化工作，
		在通常情况下，一些entity不被任何客户端所观察到的时候， 他们不需要做任何工作， 利用此接口
		可以在适当的时候激活或者停止这个entity的任意行为。
		@param isWitnessed	: 为false时， entity脱离了任何观察者的观察
		"""
		INFO_MSG("%s::onWitnessed: %i isWitnessed=%i." % (self.getScriptName(), self.id, isWitnessed))
		
		if isWitnessed:
			self.enable()
			
	def onThinkFree(self):
		"""
		virtual method.
		闲置时think
		"""
		if self.territoryControllerID <= 0:
			self.addTerritory()
		
		self.randomWalk(self.spawnPos)

	def onThinkFight(self):
		"""
		virtual method.
		战斗时think
		"""
		if self.territoryControllerID > 0:
			self.delTerritory()
		
		self.checkEnemys()
		
		if self.targetID <= 0:
			return
		
		entity = KBEngine.entities.get(self.targetID)
		if entity.position.distTo(self.position) > 2.0:
			self.gotoEntity(self.targetID, 1.8)
			return
		else:
			entity.recvDamage(self.id, 1, 0, random.randint(0, 10))
			
	def onThinkOther(self):
		"""
		virtual method.
		其他时think
		"""
		pass
		
	def onForbidChanged_(self, forbid, isInc):
		"""
		virtual method.
		entity禁止 条件改变
		@param isInc		:	是否是增加
		"""
		pass

	def onStateChanged_(self, oldstate, newstate):
		"""
		virtual method.
		entity状态改变了
		"""
		pass

	def onSubStateChanged_(self, oldSubState, newSubState):
		"""
		virtual method.
		子状态改变了
		"""
		#INFO_MSG("%i oldSubstate=%i to newSubstate=%i" % (self.id, oldSubState, newSubState))
		pass

	def onFlagsChanged_(self, flags, isInc):
		"""
		virtual method.
		"""
		pass
	
	def onEnterTrap(self, entityEntering, range_xz, range_y, controllerID, userarg):
		"""
		KBEngine method.
		有entity进入trap
		"""
		if controllerID != self.territoryControllerID:
			return
		
		if entityEntering.isDestroyed or entityEntering.getScriptName() != "Avatar" or entityEntering.isDead():
			return
		
		if not self.isState(GlobalDefine.ENTITY_STATE_FREE):
			return
			
		DEBUG_MSG("%s::onEnterTrap: %i entityEntering=(%s)%i, range_xz=%s, range_y=%s, controllerID=%i, userarg=%i" % \
						(self.getScriptName(), self.id, entityEntering.getScriptName(), entityEntering.id, \
						range_xz, range_y, controllerID, userarg))
		
		self.addEnemy(entityEntering.id, 0)

	def onLeaveTrap(self, entityLeaving, range_xz, range_y, controllerID, userarg):
		"""
		KBEngine method.
		有entity离开trap
		"""
		if controllerID != self.territoryControllerID:
			return
		
		if entityLeaving.getScriptName() != "Avatar":
			return
			
		INFO_MSG("%s::onLeaveTrap: %i entityLeaving=(%s)%i." % (self.getScriptName(), self.id, \
				entityLeaving.getScriptName(), entityLeaving.id))

	def onAddEnemy(self, entityID):
		"""
		virtual method.
		有敌人进入列表
		"""
		if not self.isState(GlobalDefine.ENTITY_STATE_FIGHT):
			self.changeState(GlobalDefine.ENTITY_STATE_FIGHT)
		
		if self.targetID == 0:
			self.setTarget(entityID)
			
	def onRemoveEnemy(self, entityID):
		"""
		virtual method.
		删除敌人
		"""
		if self.targetID == entityID:
			self.onLoseTarget()

	def onLoseTarget(self):
		"""
		敌人丢失
		"""
		INFO_MSG("%s::onLoseTarget: %i target=%i, enemyLogSize=%i." % (self.getScriptName(), self.id, \
				self.targetID, len(self.enemyLog)))
				
		self.targetID = 0
		
		if len(self.enemyLog) == 0:
			if not self.isState(GlobalDefine.ENTITY_STATE_FREE):
				self.changeState(GlobalDefine.ENTITY_STATE_FREE)
				
			self.backSpawnPos()
		else:
			self.choiceTarget()
		
AI._timermap = {}
AI._timermap[wtimer.TIMER_TYPE_HEARDBEAT] = AI.onHeardTimer