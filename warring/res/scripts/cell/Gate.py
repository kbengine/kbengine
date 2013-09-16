# -*- coding: utf-8 -*-
import random
import math
import time
import KBEngine
from KBEDebug import *
from interfaces.GameObject import GameObject

class Gate(GameObject):
	def __init__(self):
		GameObject.__init__(self) 
			
Gate._timermap = {}
Gate._timermap.update(GameObject._timermap)
