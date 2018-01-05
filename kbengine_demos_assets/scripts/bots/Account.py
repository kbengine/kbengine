# -*- coding: utf-8 -*-
import KBEngine
import copy
from KBEDebug import *

class Account(KBEngine.Entity):
	def __init__(self):
		KBEngine.Entity.__init__(self)
		DEBUG_MSG("Account::__init__:%s." % (self.__dict__))
		self.base.reqAvatarList()
		
	def onReqAvatarList(self, infos):
		"""
		defined method.
		"""
		
		DEBUG_MSG("Account:onReqAvatarList::%s" % (list(infos['values'])))
		self.base.reqCreateAvatar(1, "kbe_bot_%s" % self.id)
		self.characters = copy.deepcopy(infos["values"])

	def onCreateAvatarResult(self, retcode, info):
		"""
		defined method.
		"""
		DEBUG_MSG("Account:onCreateAvatarResult::%s, retcode=%i" % (dict(info), retcode))
		
		if retcode == 0:
			self.base.selectAvatarGame(info["dbid"])
		else:
			if len(self.characters) > 0:
				for infos in self.characters:
					self.base.selectAvatarGame(infos["dbid"])
					break

	def onRemoveAvatar(self, dbid):
		"""
		defined method.
		"""
		DEBUG_MSG("Account:onRemoveAvatar:: dbid=%i" % (dbid))
