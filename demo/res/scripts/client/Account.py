# -*- coding: utf-8 -*-
import KBEngine
from KBEDebug import *

class Account(KBEngine.Entity):
	def __init__(self):
		KBEngine.Entity.__init__(self)
		DEBUG_MSG("Account::__init__:%s." % (self.__dict__))
		self.base.reqAvatarList()
		self.avatars = {}
		
	def onReqAvatarList(self, infos):
		"""
		define method.
		"""
		DEBUG_MSG("Account:onReqAvatarList::%s" % (infos))
		self.avatars = infos
		KBEngine.fireEvent("update_avatars", self.avatars)
		
	def onCreateAvatarResult(self, retcode, info):
		"""
		define method.
		"""
		DEBUG_MSG("Account:onCreateAvatarResult::%s, retcode=%i" % (info, retcode))
		self.avatars.update(info)
		KBEngine.fireEvent("update_avatars", self.avatars)
		
	def reqCreateAvatar(self, roleType, name):
		"""
		"""
		DEBUG_MSG("Account:reqCreateAvatar::roleType=%i, name=%s" % (roleType, name))
		self.base.reqCreateAvatar(roleType, name)
