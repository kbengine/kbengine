# -*- coding: utf-8 -*-
import KBEngine
import GlobalDefine
from KBEDebug import *
from interfaces.GameObject import GameObject
from interfaces.Combat import Combat
from interfaces.Spell import Spell
from interfaces.Teleport import Teleport
from interfaces.Dialog import Dialog
from interfaces.State import State
from interfaces.Flags import Flags
from interfaces.Motion import Motion
from interfaces.SkillBox import SkillBox

class Avatar(KBEngine.Entity,
			GameObject, 
			Flags,
			State,
			Motion, 
			SkillBox,
			Combat, 
			Spell, 
			Teleport,
			Dialog):
	def __init__(self):
		KBEngine.Entity.__init__(self)
		GameObject.__init__(self) 
		Flags.__init__(self) 
		State.__init__(self) 
		Motion.__init__(self) 
		SkillBox.__init__(self) 
		Combat.__init__(self) 
		Spell.__init__(self) 
		Teleport.__init__(self) 
		Dialog.__init__(self) 
		
		# 设置每秒允许的最快速度, 超速会被拉回去
		self.topSpeed = self.moveSpeed + 5.0
		# self.topSpeedY = 10.0
		
		# 如果为7说明在UE4地图中，那么为了配合demo的移动速度，我们将限制设置得更大一些
		if self.spaceUType == 7:
			self.topSpeed = 0

	def isPlayer(self):
		"""
		virtual method.
		"""
		return True
		
	def startDestroyTimer(self):
		"""
		virtual method.
		
		启动销毁entitytimer
		"""
		pass

	#--------------------------------------------------------------------------------------------
	#                              Callbacks
	#--------------------------------------------------------------------------------------------
	def onTimer(self, tid, userArg):
		"""
		KBEngine method.
		引擎回调timer触发
		"""
		#DEBUG_MSG("%s::onTimer: %i, tid:%i, arg:%i" % (self.getScriptName(), self.id, tid, userArg))
		GameObject.onTimer(self, tid, userArg)
		Spell.onTimer(self, tid, userArg)
		
	def onGetWitness(self):
		"""
		KBEngine method.
		绑定了一个观察者(客户端)
		"""
		DEBUG_MSG("Avatar::onGetWitness: %i." % self.id)

	def onLoseWitness(self):
		"""
		KBEngine method.
		解绑定了一个观察者(客户端)
		"""
		DEBUG_MSG("Avatar::onLoseWitness: %i." % self.id)
	
	def onDestroy(self):
		"""
		KBEngine method.
		entity销毁
		"""
		DEBUG_MSG("Avatar::onDestroy: %i." % self.id)
		Teleport.onDestroy(self)
		Combat.onDestroy(self)
		
	def relive(self, exposed, type):
		"""
		defined.
		复活
		"""
		if exposed != self.id:
			return
			
		DEBUG_MSG("Avatar::relive: %i, type=%i." % (self.id, type))
		
		# 回城复活
		if type == 0:
			pass
			
		self.fullPower()
		self.changeState(GlobalDefine.ENTITY_STATE_FREE)

	def jump(self, exposed):
		"""
		defined.
		玩家跳跃 我们广播这个行为
		"""
		if exposed != self.id:
			return
		
		self.otherClients.onJump()
		
	def onAddEnemy(self, entityID):
		"""
		virtual method.
		有敌人进入列表
		"""
		if not self.isState(GlobalDefine.ENTITY_STATE_FIGHT):
			self.changeState(GlobalDefine.ENTITY_STATE_FIGHT)

	def onEnemyEmpty(self):
		"""
		virtual method.
		敌人列表空了
		"""
		self.changeState(GlobalDefine.ENTITY_STATE_FREE)
