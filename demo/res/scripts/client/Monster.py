# -*- coding: utf-8 -*-
import KBEngine
import KBExtra
from KBEDebug import *
from interfaces.GameObject import GameObject
from interfaces.Motion import Motion

class Monster(GameObject, Motion):
	def __init__(self):
		GameObject.__init__(self)
		Motion.__init__(self)
