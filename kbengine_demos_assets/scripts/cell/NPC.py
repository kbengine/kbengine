# -*- coding: utf-8 -*-
import random
import math
import time
import KBEngine
from KBEDebug import *
from interfaces.NPCObject import NPCObject
from interfaces.Motion import Motion

class NPC(KBEngine.Entity, NPCObject, Motion):
	def __init__(self):
		KBEngine.Entity.__init__(self)
		NPCObject.__init__(self)
		Motion.__init__(self)

	def isNPC(self):
		"""
		virtual method.
		"""
		return True

	#--------------------------------------------------------------------------------------------
	#                              Callbacks
	#--------------------------------------------------------------------------------------------
	def onTimer(self, tid, userArg):
		"""
		KBEngine method.
		引擎回调timer触发
		"""
		#DEBUG_MSG("%s::onTimer: %i, tid:%i, arg:%i" % (self.getScriptName(), self.id, tid, userArg))
		NPCObject.onTimer(self, tid, userArg)
		
	def onDestroy(self):
		"""
		entity销毁
		"""
		NPCObject.onDestroy(self)
