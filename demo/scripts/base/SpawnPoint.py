# -*- coding: utf-8 -*-
import KBEngine
from KBEDebug import *
from interfaces.GameObject import GameObject

class SpawnPoint(KBEngine.Base, GameObject):
	def __init__(self):
		KBEngine.Base.__init__(self)
		GameObject.__init__(self)
		self.createCellEntity(self.createToCell)
		
SpawnPoint._timermap = {}
SpawnPoint._timermap.update(GameObject._timermap)
