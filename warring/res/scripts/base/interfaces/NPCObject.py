# -*- coding: utf-8 -*-
import KBEngine
from KBEDebug import *
from interfaces.GameObject import GameObject

class NPCObject(GameObject):
	def __init__(self):
		GameObject.__init__(self)
		
NPCObject._timermap = {}
NPCObject._timermap.update(GameObject._timermap)