# -*- coding: utf-8 -*-
import KBEngine
import random
from AVATAR_INFOS import TAvatarInfosList
from KBEDebug import *
import d_avatar_inittab

class Account(KBEngine.Proxy):
	def __init__(self):
		KBEngine.Proxy.__init__(self)
		self.activeCharacter = None
		
	def onTimer(self, id, userArg):
		"""
		KBEngine method.
		使用addTimer后， 当时间到达则该接口被调用
		@param id		: addTimer 的返回值ID
		@param userArg	: addTimer 最后一个参数所给入的数据
		"""
		DEBUG_MSG(id, userArg)
		
	def onEntitiesEnabled(self):
		"""
		KBEngine method.
		该entity被正式激活为可使用， 此时entity已经建立了client对应实体， 可以在此创建它的
		cell部分。
		"""
		INFO_MSG("account[%i] entities enable. mailbox:%s" % (self.id, self.client))
		self.writeToDB()
		
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
		if self.activeCharacter:
			self.activeCharacter.accountEntity = None
			self.activeCharacter = None
		DEBUG_MSG("Account[%i].onClientDeath:" % self.id)
		self.destroy()
	
	def reqAvatarList(self):
		"""
		exposed.
		客户端请求查询角色列表
		"""
		DEBUG_MSG("Account[%i].reqAvatarList:" % self.id)
		listdata = TAvatarInfosList()
		listdata.update(self.characters)
		avatarinfo = ["unknown",  0, 0]
		characterssize = len(listdata)
		
		if characterssize < 3:
			for x in range(0, 3-characterssize):
				listdata[x] = avatarinfo

		self.client.onReqAvatarList(listdata)
				
	def reqCreateAvatar(self, roleType, name):
		"""
		exposed.
		客户端请求创建一个角色
		"""
		avatarinfo = {"name": name, "dbid": 0, "roleType" : roleType, "level" : 0}
			
		"""
		if name in all_avatar_names:
			retcode = 2
			self.client.onCreateAvatarResult(retcode, avatarinfo)
			return
		"""
		
		if len(self.characters) >= 3:
			DEBUG_MSG("Account[%i].reqCreateAvatar:%s. character=%s.\n" % (self.id, name, self.characters))
			self.client.onCreateAvatarResult(3, avatarinfo)
			return
			
		props = {
			"name"				: name,
			"roleType"			: roleType,
			"level"				: 1,
			"spaceUType"		: d_avatar_inittab.datas[roleType]["spaceUType"],
			"direction"			: (0, 0, d_avatar_inittab.datas[roleType]["spawnYaw"]),
			"position"			: d_avatar_inittab.datas[roleType]["spawnPos"]
			}
			
		avatar = KBEngine.createBaseLocally('Avatar', props)
		if avatar:
			avatar.writeToDB(self._onCharacterSaved)
		
	def _onCharacterSaved(self, success, avatar):
		"""
		新建角色写入数据库回调
		"""
		INFO_MSG('Account::_onCharacterSaved:(%i) create avatar state: %i, %s, %i' % (self.id, success, avatar.cellData["name"], avatar.databaseID))
		avatarinfo = {"name": "", "dbid": 0, "roleType" : 0, "level" : 0}
		
		if success:
			self.characters[avatar.databaseID] = [avatar.cellData["name"], avatar.roleType, 1]
			avatarinfo["dbid"] = avatar.databaseID
			avatarinfo["name"] = avatar.cellData["name"]
			avatarinfo["roleType"] = avatar.roleType
			avatarinfo["level"] = 1
			self.writeToDB()

			avatar.destroy()
		
		self.client.onCreateAvatarResult(0, avatarinfo)
			
	def selectAvatarGame(self, dbid):
		"""
		exposed.
		客户端选择某个角色进行游戏
		"""
		DEBUG_MSG("Account[%i].selectAvatarGame:%i. self.activeCharacter=%s" % (self.id, dbid, self.activeCharacter))
		# 注意:使用giveClientTo的entity必须是当前baseapp上的entity
		if self.activeCharacter is None:
			if dbid in self.characters:
				player = KBEngine.createBaseFromDBID("Avatar", dbid, self.__onAvatarCreated)
			else:
				ERROR_MSG("Account::selectAvatarGame: not found dbid(%i)" % dbid)
		else:
			self.giveClientTo(self.activeCharacter)
			
	def __onAvatarCreated(self, baseRef, dbid, wasActive):
		"""
		选择角色进入游戏时被调用
		"""
		if wasActive:
			ERROR_MSG("(%i): this character is in world now!" % (self.id))
			return
		if baseRef is None:
			ERROR_MSG("(%i): the character you wanted to created is not exist!" % (self.id))
			return
			
		avatar = KBEngine.entities.get(baseRef.id)
		if avatar is None:
			ERROR_MSG("(%i): when character was created, it died as well!" % (self.id))
			return

		info = self.characters[dbid]
		avatar.cellData["headID"] = d_avatar_inittab.datas[info[1]]["headResID"]
		avatar.cellData["modelID"] = d_avatar_inittab.datas[info[1]]["modelResID"]
												
		avatar.accountEntity = self
		self.activeCharacter = avatar
		self.giveClientTo(avatar)
		