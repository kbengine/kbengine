# -*- coding: utf-8 -*-
import KBEngine
import random
from KBEDebug import *
from Space import Space
import d_entities
import d_spaces

class SpaceDuplicate(Space):
	def __init__(self):
		Space.__init__(self)
		
SpaceDuplicate._timermap = {}
SpaceDuplicate._timermap.update(Space._timermap)
