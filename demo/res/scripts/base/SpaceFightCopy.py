# -*- coding: utf-8 -*-
import KBEngine
import random
from KBEDebug import *
from Space import Space
import d_entities
import d_spaces

class SpaceFightCopy(Space):
	def __init__(self):
		Space.__init__(self)
		
	def startInputFigth(self):
		"""
		defined method.
		"""
		for e in self.avatars:
			DEBUG_MSG("SpaceFightCopy::startInputFigth(%i)" % e.id)
			e.client.startInputFigth(30)