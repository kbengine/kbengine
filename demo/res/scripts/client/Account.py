# -*- coding: utf-8 -*-
import KBEngine
from KBEDebug import *

class Account(KBEngine.Entity):
	def __init__(self):
		KBEngine.Entity.__init__(self)
		DEBUG_MSG("my account name=%s." % self.accountName)
		
	def initPlayerList(self, playerList): 
		"""
		define method.
		"""
		DEBUG_MSG(playerList)
