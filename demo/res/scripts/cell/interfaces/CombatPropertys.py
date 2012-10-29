# -*- coding: utf-8 -*-
import KBEngine
from KBEDebug import * 

class CombatPropertys:
	def __init__(self):
		self.HP_Max = 100
		self.MP_Max = 100
		
		self.HP = self.HP_Max
		self.MP = self.MP_Max
		
CombatPropertys._timermap = {}
