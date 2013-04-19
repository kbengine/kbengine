# -*- coding: utf-8 -*-
import random
import math
import time
import KBEngine
from KBEDebug import *
from interfaces.GameObject import GameObject
from interfaces.Motion import Motion

class NPC(GameObject, Motion):
	def __init__(self):
		GameObject.__init__(self)
		Motion.__init__(self)
		
NPC._timermap = {}
NPC._timermap.update(GameObject._timermap)
NPC._timermap.update(Motion._timermap)