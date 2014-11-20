# -*- coding: utf-8 -*-
import KBEngine
from KBEDebug import * 

class CombatPropertys:
	def __init__(self):
		self.HP_Max = 100
		self.MP_Max = 100
		
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
		
CombatPropertys._timermap = {}
