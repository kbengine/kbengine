# -*- coding: utf-8 -*-
import KBEngine
import random
from KBEDebug import *
from SpaceCopy import SpaceCopy
import d_entities
import d_spaces

class SpaceFightCopy(SpaceCopy):
	def __init__(self):
		SpaceCopy.__init__(self)
			
SpaceFightCopy._timermap = {}
SpaceFightCopy._timermap.update(SpaceCopy._timermap)
