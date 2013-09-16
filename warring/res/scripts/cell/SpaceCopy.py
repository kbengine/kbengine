# -*- coding: utf-8 -*-
import KBEngine
import random
from KBEDebug import *
from Space import Space
import d_entities
import d_spaces

class SpaceCopy(Space):
	def __init__(self):
		Space.__init__(self)
		self.avatars = {}

	def onEnter(self, entityMailbox):
		"""
		defined method.
		进入场景
		"""
		self.avatars[entityMailbox.id] = entityMailbox
		Space.onEnter(self, entityMailbox)
		
	def onLeave(self, entityID):
		"""
		defined method.
		离开场景
		"""
		if entityID in self.avatars:
			del self.avatars[entityID]
		
		Space.onLeave(self, entityID)
		
SpaceCopy._timermap = {}
SpaceCopy._timermap.update(Space._timermap)
