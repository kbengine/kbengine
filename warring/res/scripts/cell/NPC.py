# -*- coding: utf-8 -*-
import random
import math
import time
import KBEngine
from KBEDebug import *
from interfaces.NPCObject import NPCObject
from interfaces.Motion import Motion

class NPC(NPCObject, Motion):
	def __init__(self):
		NPCObject.__init__(self)
		Motion.__init__(self)

	def onDestroy(self):
		"""
		entity销毁
		"""
		NPCObject.onDestroy(self)
		
NPC._timermap = {}
NPC._timermap.update(NPCObject._timermap)
NPC._timermap.update(Motion._timermap)