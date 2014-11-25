# -*- coding: utf-8 -*-
import KBEngine
from KBEDebug import *
import json
import copy

class Account(KBEngine.Entity):
	def __init__(self):
		KBEngine.Entity.__init__(self)
		DEBUG_MSG("Account::__init__:%s." % (self.__dict__))
		self.base.reqAvatarList()
		self.avatars = {}
		
	def onReqAvatarList(self, infos):
		"""
		defined method.
		"""
		DEBUG_MSG("Account:onReqAvatarList::%s" % (infos))
		
		self.avatars = infos
		self.fireEvent("update_avatars")
	
	def fireEvent(self, evtName):
		firedatas = ""
		
		if evtName == "update_avatars":
			dctinfo = copy.deepcopy(dict(self.avatars))
			for info in dctinfo.values():
				for data in info[4].values():
					data[1] = ""
			
			firedatas = json.dumps(dctinfo)
			
		KBEngine.fireEvent(evtName, firedatas)
			
	def onCreateAvatarResult(self, retcode, info):
		"""
		defined method.
		"""
		DEBUG_MSG("Account:onCreateAvatarResult::%s, retcode=%i" % (info, retcode))
		
		if info[0] == 0: # "dbid"
			DEBUG_MSG("Account:onCreateAvatarResult::avatar full.")
			return
			
		self.avatars[info[0]] = info
		self.fireEvent("update_avatars")
	
	def onRemoveAvatar(self, dbid):
		"""
		defined method.
		"""
		DEBUG_MSG("Account:onRemoveAvatar:: dbid=%i" % (dbid))
		del self.avatars[dbid]
		
	def reqCreateAvatar(self, strargs):
		"""
		"""
		roleType, name = json.loads(strargs)
		DEBUG_MSG("Account:reqCreateAvatar::roleType=%i, name=%s" % (roleType, name))
		self.base.reqCreateAvatar(roleType, name)

	def selectAvatarGame(self, strargs):
		"""
		选择某个角色进行游戏
		"""
		dbid = json.loads(strargs)
		DEBUG_MSG("Account:selectAvatarGame::dbid=%s" % (dbid))
		self.base.selectAvatarGame(dbid)

