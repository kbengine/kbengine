# -*- coding: utf-8 -*-
import KBEngine
import KBExtra
import random, math
import Math
import time
import d_spaces
import GlobalDefine
from KBEDebug import *
from interfaces.GameObject import GameObject
from interfaces.Dialog import Dialog
from interfaces.Teleport import Teleport
from interfaces.State import State
from interfaces.Flags import Flags
from interfaces.Combat import Combat
from interfaces.Spell import Spell
from interfaces.SkillBox import SkillBox
from interfaces.Motion import Motion

class Avatar(KBEngine.Entity,
			GameObject,
			Flags,
			State,
			SkillBox,
			Combat, 
			Spell, 
			Dialog,
			Motion,
			Teleport):
	def __init__(self):
		KBEngine.Entity.__init__(self)
		GameObject.__init__(self)
		Flags.__init__(self) 
		State.__init__(self) 
		SkillBox.__init__(self) 
		Combat.__init__(self) 
		Spell.__init__(self) 
		Dialog.__init__(self)
		Motion.__init__(self)
		Teleport.__init__(self)
		
	def onEnterSpace(self):
		"""
		KBEngine method.
		这个entity进入了一个新的space
		"""
		DEBUG_MSG("%s::onEnterSpace: %i" % (self.__class__.__name__, self.id))

	def onLeaveSpace(self):
		"""
		KBEngine method.
		这个entity将要离开当前space
		"""
		DEBUG_MSG("%s::onLeaveSpace: %i" % (self.__class__.__name__, self.id))
		
	def onBecomePlayer( self ):
		"""
		KBEngine method.
		当这个entity被引擎定义为角色时被调用
		"""
		DEBUG_MSG("%s::onBecomePlayer: %i" % (self.__class__.__name__, self.id))
		
	def onJump(self):
		"""
		defined method.
		玩家跳跃
		"""
		pass
		
	def update(self):
		pass

class PlayerAvatar(Avatar):
	def __init__(self):
		self.randomWalkRadius = 10.0
		self.reliveTime = -1
		self.testTeleportPos = None
		self.attackTargetID = 0
		self.testType = random.randint(0, 2) # 测试类别， 0：随机移动， 1：找目标攻击， 2：测试传送
		self.changeTestTypeTime = time.time()
		self.spawnPosition = Math.Vector3( self.position )

	def onBecomePlayer( self ):
		"""
		KBEngine method.
		当这个entity被引擎定义为角色时被调用
		"""
		DEBUG_MSG("%s::onBecomePlayer: %i" % (self.__class__.__name__, self.id))
		
		# 注意：由于PlayerAvatar是引擎底层强制由Avatar转换过来，__init__并不会再调用
		# 这里手动进行初始化一下
		self.__init__()
		
		KBEngine.callback(1, self.update)

	def onEnterSpace(self):
		"""
		KBEngine method.
		这个entity进入了一个新的space
		"""
		DEBUG_MSG("%s::onEnterSpace: %i" % (self.__class__.__name__, self.id))
		
		# 注意：由于PlayerAvatar是引擎底层强制由Avatar转换过来，__init__并不会再调用
		# 这里手动进行初始化一下
		self.__init__()
		
	def onLeaveSpace(self):
		"""
		KBEngine method.
		这个entity将要离开当前space
		"""
		DEBUG_MSG("%s::onLeaveSpace: %i" % (self.__class__.__name__, self.id))

	def calcRandomWalkPosition( self ):
		"""
		计算随机移动位置
		"""
		center = self.spawnPosition
		r = random.uniform( 1, self.randomWalkRadius ) # 最少走1米
		b = 360.0 * random.random()
		x = r * math.cos( b )		# 半径 * 正余玄
		z = r * math.sin( b )
		return Math.Vector3( center.x + x, center.y, center.z + z )

	def testAttackTarget(self):
		if self.attackTargetID not in self.clientapp.entities:

			self.attackTargetID = 0

			# 从列表中随机找一个怪物攻击
			for id, e in self.clientapp.entities.items():
				if e.className == 'Monster':
					self.attackTargetID = id
					break

			# 找不到怪攻击就找人攻击
			if self.attackTargetID == 0:
				if self.clientapp.getSpaceData("_mapping") == 'spaces/cell_sene':
					for id, e in self.clientapp.entities.items():
						if e.className == 'Avatar' and id != self.id and not e.isState(GlobalDefine.ENTITY_STATE_DEAD):
							self.attackTargetID = id
							break

			if self.attackTargetID == 0:
				return

		targetPos = self.clientapp.entities[self.attackTargetID].position
		
		if self.position.distTo(targetPos) > self.velocity:
			self.moveToPoint( targetPos, self.velocity, 0.0, 0, True, True )
		else:
			self.cell.useTargetSkill(1, self.attackTargetID)

	def testTeleport(self):
			targetPos = self.testTeleportPos

			if targetPos != None:
				self.moveToPoint( targetPos, self.velocity, 0.0, 0, True, True )
			
			# 有概率的做传送测试, 只在u3ddemo中测试
			if self.testTeleportPos == None and random.randint(0, 10) < 1:
				if self.clientapp.getSpaceData("_mapping") == 'spaces/cell_sene':
					
					if random.randint(0, 10) < 5:
						# 本地传送
						self.testTeleportPos = (-20.340378, 1.5, -150.070831)
					else:
						# 跨场景传送
						self.testTeleportPos = (-34.340378, 1.5, -121.070831)

				if self.clientapp.getSpaceData("_mapping") == 'spaces/duplicate':
					self.testTeleportPos = (10, 1.5, 0)
			
			if self.testTeleportPos != None:
				if self.position.distTo(self.testTeleportPos) < self.velocity:
					self.testTeleportPos = None
	
	def updateTest(self):
		# 隔一段时间换一种测试方式
		if time.time() - self.changeTestTypeTime > 60.0:
			self.changeTestTypeTime = time.time()
			self.testType = random.randint(0, 2)
				
	def update(self):
		#DEBUG_MSG("%s::update: %i" % (self.__class__.__name__, self.id))
		if self.isDestroyed:
			return

		KBEngine.callback(1, self.update)
		
		# 如果自己已经死亡了，那么延时一下复活
		if self.isState(GlobalDefine.ENTITY_STATE_DEAD):
			if self.reliveTime == -1:
				self.reliveTime = random.randint(1, 10)
			elif self.reliveTime > 0:
				self.reliveTime -= 1
			else:
				self.cell.relive(1)

			return
		else:
			self.reliveTime = -1

			self.updateTest()

			if self.testType == 1:
				self.testAttackTarget()
			elif self.testType == 2:
				self.testTeleport()
			else:
				self.moveToPoint( self.calcRandomWalkPosition(), self.velocity, 0.0, 0, True, True )