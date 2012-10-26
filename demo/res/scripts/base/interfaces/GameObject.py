# -*- coding: utf-8 -*-
import KBEngine
from KBEDebug import *

class GameObject(KBEngine.Base):
	def __init__(self):
		KBEngine.Base.__init__(self)

	def getScriptName(self):
		return self.__class__.__name__
		
	def onTimer(self, tid, userArg):
		#DEBUG_MSG("%s::onTimer: %i, tid:%i, arg:%i" % (self.getScriptName(), self.id, tid, userArg))
		self._timermap[userArg](self, tid, userArg)

GameObject._timermap = {}
