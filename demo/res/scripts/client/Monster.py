# -*- coding: utf-8 -*-
import KBEngine
import KBExtra
from KBEDebug import *
from interfaces.GameObject import GameObject
from interfaces.Motion import Motion
from interfaces.State import State
from interfaces.Flags import Flags

class Monster(GameObject, 
			Flags,
			State,
			Motion):
	def __init__(self):
		GameObject.__init__(self)
		Motion.__init__(self)
		Flags.__init__(self) 
		State.__init__(self) 
