# -*- coding: utf-8 -*-
import KBEngine
import KBExtra
from KBEDebug import *
from interfaces.GameObject import GameObject
from interfaces.Motion import Motion
from interfaces.State import State
from interfaces.Flags import Flags
from interfaces.Combat import Combat

class Monster(KBEngine.Entity,
			GameObject, 
			Flags,
			State,
			Motion,
			Combat):
	def __init__(self):
		KBEngine.Entity.__init__(self)
		GameObject.__init__(self)
		Motion.__init__(self)
		Flags.__init__(self) 
		State.__init__(self) 
		Combat.__init__(self) 
