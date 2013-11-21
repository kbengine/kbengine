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

class Avatar(GameObject, 
			Flags,
			State,
			Motion, 
			Combat, 
			Spell, 
			Teleport,
			Dialog):
	def __init__(self):
		GameObject.__init__(self) 
		Flags.__init__(self) 
		State.__init__(self) 
		Motion.__init__(self) 
		Combat.__init__(self) 
		Spell.__init__(self) 
		Teleport.__init__(self) 
		Dialog.__init__(self) 
		
		# 设置每秒允许的最快速度, 超速会被拉回去
		self.topSpeed = 10.0
		# self.topSpeedY = 10.0

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
		
Avatar._timermap = {}
Avatar._timermap.update(GameObject._timermap)
Avatar._timermap.update(Flags._timermap)
Avatar._timermap.update(State._timermap)
Avatar._timermap.update(Motion._timermap)
Avatar._timermap.update(Combat._timermap)
Avatar._timermap.update(Spell._timermap)
Avatar._timermap.update(Teleport._timermap)
Avatar._timermap.update(Dialog._timermap)
