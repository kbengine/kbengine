# -*- coding: utf-8 -*-
import KBEngine
import KBExtra
from KBEDebug import *
from interfaces.GameObject import GameObject

class NPC(KBEngine.Entity, GameObject):
	def __init__(self):
		KBEngine.Entity.__init__(self)
		GameObject.__init__(self)
