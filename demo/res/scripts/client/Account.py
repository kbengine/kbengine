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
		
		if info[0] == 0: # "dbid"
			DEBUG_MSG("Account:onCreateAvatarResult::avatar full.")
			return
			
		self.avatars[info[0]] = info
		KBEngine.fireEvent("update_avatars", self.avatars)
	
	def onRemoveAvatar(self, dbid):
		"""
		define method.
		"""
		DEBUG_MSG("Account:onRemoveAvatar:: dbid=%i" % (dbid))
		del self.avatars[dbid]
		
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

