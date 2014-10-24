# -*- coding: utf-8 -*-
import KBEngine
from KBEDebug import *
from interfaces.GameObject import GameObject
INFO_MSG(str.format('exec file: {}....', __file__))
class NPCObject(GameObject):
	def __init__(self):
		GameObject.__init__(self)
		
NPCObject._timermap = {}
NPCObject._timermap.update(GameObject._timermap)