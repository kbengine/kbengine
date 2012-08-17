# -*- coding: utf-8 -*-
import KBEngine
from KBEDebug import *

all_avatar_names = []

class Account(KBEngine.Proxy):
	def __init__(self):
		KBEngine.Proxy.__init__(self)
		self.avatars = [{"name":"unknown", "dbid": 0}, {"name":"unknown", "dbid": 0}, {"name":"unknown", "dbid": 0}]
		self.accountName = "kebiao"
		self.activeCharacter = None
		
	def onTimer(self, id, userArg):
		"""
		KBEngine method.
		使用addTimer后， 当时间到达则该接口被调用
		@param id		: addTimer 的返回值ID
		@param userArg	: addTimer 最后一个参数所给入的数据
		"""
		DEBUG_MSG(id, userArg)
		#self.giveClientTo(None)
		#self.destroy()
		
	def onEntitiesEnabled(self):
		"""
		KBEngine method.
		该entity被正式激活为可使用， 此时entity已经建立了client对应实体， 可以在此创建它的
		cell部分。
		"""
		INFO_MSG("account[%i] entities enable. mailbox:%s" % (self.id, self.client))
		
		#self.client.initPlayerList(self.avatars)
		
	def onLogOnAttempt(self, ip, port, password):
		"""
		KBEngine method.
		客户端登陆失败时会回调到这里
		"""
		INFO_MSG(ip, port, password)
		"""
		if self.activeCharacter != None:
			return KBEngine.LOG_ON_REJECT

		if ip == self.lastClientIpAddr and password == self.password:
			return KBEngine.LOG_ON_ACCEPT
		else:
			return KBEngine.LOG_ON_REJECT
		"""
		if self.activeCharacter:
			self.activeCharacter.giveClientTo(self)
		return KBEngine.LOG_ON_ACCEPT
		
	def onClientDeath(self):
		"""
		KBEngine method.
		客户端对应实体已经销毁
		"""
		DEBUG_MSG("Account[%i].onClientDeath:" % self.id)
		self.destroy()

	def reqAvatarList(self):
		"""
		exposed.
		客户端请求查询角色列表
		"""
		DEBUG_MSG("Account[%i].reqAvatarList:" % self.id)
		self.client.onReqAvatarList(self.avatars)
		
	def reqCreateAvatar(self, name):
		"""
		exposed.
		客户端请求创建一个角色
		"""
		avatarinfo = {"name": name, "dbid": id(name)}
			
		if name in all_avatar_names:
			retcode = 2
			self.client.onCreateAvatarResult(retcode, avatarinfo)
			return
			
		done = False
		for info in self.avatars:
			if info["dbid"] == 0:
				info["dbid"] = avatarinfo["dbid"]
				info["name"] = avatarinfo["name"]
				all_avatar_names.append(name)
				done = True
				break
			
		retcode = 0
		
		if not done:
			retcode = 3
			
		DEBUG_MSG("Account[%i].reqCreateAvatar:%s. avatars=%s.\n" % (self.id, name, self.avatars))
		self.client.onCreateAvatarResult(retcode, avatarinfo)
		
	def selectAvatarGame(self, dbid):
		"""
		exposed.
		客户端选择某个角色进行游戏
		"""
		DEBUG_MSG("Account[%i].selectAvatarGame:%i." % (self.id, dbid))
		# 注意:使用giveClientTo的entity必须是当前baseapp上的entity
		if self.activeCharacter is None:
			player = KBEngine.createBase("Avatar", {"name" : str(dbid)})
			player.accountEntity = self
			self.giveClientTo(player)