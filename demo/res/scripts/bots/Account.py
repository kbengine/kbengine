# -*- coding: utf-8 -*-
import KBEngine
from KBEDebug import *

class Account(KBEngine.Entity):
	def __init__(self):
		KBEngine.Entity.__init__(self)
		DEBUG_MSG("my account name=%s." % self.accountName)

	def onReqAvatarList(self, infos):
		"""
		define method.
		"""
		DEBUG_MSG("Account:onReqAvatarList::%s" % (infos))
		
	def onCreateAvatarResult(self, retcode, info):
		"""
		define method.
		"""
		DEBUG_MSG("Account:onCreateAvatarResult::%s, retcode=%i" % (info, retcode))

