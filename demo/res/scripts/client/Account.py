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
		DEBUG_MSG("Account:onReqAvatarList::%s" % (dict(infos)))
		self.avatars = dict(infos)
		KBEngine.fireEvent("update_avatars", self.avatars)
		
	def onCreateAvatarResult(self, retcode, info):
		"""
		define method.
		"""
		DEBUG_MSG("Account:onCreateAvatarResult::%s, retcode=%i" % (dict(info), retcode))
		
		if info["dbid"] == 0:
			DEBUG_MSG("Account:onCreateAvatarResult::avatar full.")
			return
			
		self.avatars[info["dbid"]] = dict(info);
		KBEngine.fireEvent("update_avatars", self.avatars)
		
	def reqCreateAvatar(self, roleType, name):
		"""
		"""
		DEBUG_MSG("Account:reqCreateAvatar::roleType=%i, name=%s" % (roleType, name))
		self.base.reqCreateAvatar(roleType, name)

	def selectAvatarGame(self, dbid):
		"""
		选择某个角色进行游戏
		"""
		DEBUG_MSG("Account:selectAvatarGame::dbid=%i" % (dbid))
		self.base.selectAvatarGame(dbid)

