# -*- coding: utf-8 -*-
import KBEngine
from KBEDebug import *

class Account(KBEngine.Proxy):
	def __init__(self):
		KBEngine.Proxy.__init__(self)
		self.playerList = [1,2]
		self.accountName = "kebiao"
		
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
		
		#self.client.initPlayerList(self.playerList)
		
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
		
	def onClientDeath(self):
		"""
		KBEngine method.
		客户端对应实体已经销毁
		"""
		DEBUG_MSG("Account[%i].onClientDeath:" % self.id)
		self.destroy()

	def onReqPlayerList(self):
		"""
		exposed.
		客户端请求查询角色列表
		"""
		DEBUG_MSG("Account[%i].onReqPlayerList:" % self.id)
	
	def reqCreateAvatar(self, name):
		"""
		exposed.
		客户端请求创建一个角色
		"""
		DEBUG_MSG("Account[%i].reqCreateAvatar:%s." % (self.id, name))
		
	def selectAvatarGame(self, name):
		"""
		exposed.
		客户端选择某个角色进行游戏
		"""
		DEBUG_MSG("Account[%i].selectAvatarGame:%s." % (self.id, name))
		# 注意:使用giveClientTo的entity必须是当前baseapp上的entity
		player = KBEngine.createBase("Avatar", {"name" : name})
		player.accountEntity = self
		self.giveClientTo(player)