# -*- coding: utf-8 -*-
import KBEngine
import GlobalDefine
from KBEDebug import * 

class CombatPropertys:
	"""
	所有关于战斗的属性
	完善的话可以根据策划excel表来直接生成这个模块
	"""
	def __init__(self):
		self.HP_Max = 100
		self.MP_Max = 100
		
		# 非死亡状态才需要补满
		if not self.isState(GlobalDefine.ENTITY_STATE_DEAD) and self.HP == 0 and self.MP == 0:
			self.fullPower()
	
	def fullPower(self):
		"""
		"""
		self.setHP(self.HP_Max)
		self.setMP(self.MP_Max)
		
	def addHP(self, val):
		"""
		defined.
		"""
		v = self.HP + int(val)
		if v < 0:
			v = 0
			
		if self.HP == v:
			return
			
		self.HP = v
			
	def addMP(self, val):
		"""
		defined.
		"""
		v = self.MP + int(val)
		if v < 0:
			v = 0
			
		if self.MP == v:
			return
			
		self.MP = v
		
	def setHP(self, hp):
		"""
		defined
		"""
		hp = int(hp)
		if hp < 0:
			hp = 0
		
		if self.HP == hp:
			return
			
		self.HP = hp

	def setMP(self, mp):
		"""
		defined
		"""
		hp = int(mp)
		if mp < 0:
			mp = 0

		if self.MP == mp:
			return
			
		self.MP = mp

	def setHPMax(self, hpmax):
		"""
		defined
		"""
		hpmax = int(hpmax)
		self.HP_Max = hpmax
			
	def setMPMax(self, mpmax):
		"""
		defined
		"""
		mpmax = int(mpmax)
		self.MP_Max = mpmax
		

