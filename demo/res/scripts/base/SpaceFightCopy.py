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
		
	def startInputFigth(self):
		"""
		defined method.
		"""
		for e in self.avatars:
			DEBUG_MSG("SpaceFightCopy::startInputFigth(%i)" % e.id)
			e.client.startInputFigth(30)
			
SpaceFightCopy._timermap = {}
SpaceFightCopy._timermap.update(SpaceCopy._timermap)
