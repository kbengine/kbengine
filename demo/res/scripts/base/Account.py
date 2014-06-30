# -*- coding: utf-8 -*-
import KBEngine
import random
import time
import d_spaces
from AVATAR_INFOS import TAvatarInfos
from AVATAR_INFOS import TAvatarInfosList
from AVATAR_DATA import TAvatarData
from KBEDebug import *
import d_avatar_inittab

class Account(KBEngine.Proxy):
	def __init__(self):
		KBEngine.Proxy.__init__(self)
		self.activeCharacter = None
		self.relogin = time.time()
		
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
		INFO_MSG("Account[%i]::onEntitiesEnabled:entities enable. mailbox:%s, clientType(%i), hasAvatar=%s, accountName=%s" % \
			(self.id, self.client, self.getClientType(), self.activeCharacter, self.__ACCOUNT_NAME__))
			
	def onLogOnAttempt(self, ip, port, password):
		"""
		KBEngine method.
		客户端登陆失败时会回调到这里
		"""
		INFO_MSG("Account[%i]::onLogOnAttempt: ip=%s, port=%i, selfclient=%s" % (self.id, ip, port, self.client))
		"""
		if self.activeCharacter != None:
			return KBEngine.LOG_ON_REJECT

		if ip == self.lastClientIpAddr and password == self.password:
			return KBEngine.LOG_ON_ACCEPT
		else:
			return KBEngine.LOG_ON_REJECT
		"""
		
		# 如果一个在线的账号被一个客户端登陆并且onLogOnAttempt返回允许
		# 那么会踢掉之前的客户端连接
		# 那么此时self.activeCharacter可能不为None， 常规的流程是销毁这个角色等新客户端上来重新选择角色进入
		if self.activeCharacter:
			self.activeCharacter.giveClientTo(self)
			self.relogin = time.time()
			self.activeCharacter.destroySelf()
			self.activeCharacter = None
			
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
		DEBUG_MSG("Account[%i].reqAvatarList: size=%i." % (self.id, len(self.characters)))
		self.client.onReqAvatarList(self.characters)
				
	def reqCreateAvatar(self, roleType, name):
		"""
		exposed.
		客户端请求创建一个角色
		"""
		avatarinfo = TAvatarInfos()
		avatarinfo.extend([0, "", 0, 0, TAvatarData().createFromDict({"param1" : 0, "param2" :b''})])
			
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
		
		""" 根据前端类别给出出生点
		UNKNOWN_CLIENT_COMPONENT_TYPE	= 0,
		CLIENT_TYPE_MOBILE				= 1,	// 手机类
		CLIENT_TYPE_PC					= 2,	// pc， 一般都是exe客户端
		CLIENT_TYPE_BROWSER				= 3,	// web应用， html5，flash
		CLIENT_TYPE_BOTS				= 4,	// bots
		CLIENT_TYPE_MINI				= 5,	// 微型客户端
		"""
		spaceUType = 1
		
		if self.getClientType() == 2:
			spaceUType = 2
		elif self.getClientType() == 5:
			spaceUType = 3
		else:
			spaceUType = 1
		
		spaceData = d_spaces.datas.get(spaceUType)
		
		props = {
			"name"				: name,
			"roleType"			: roleType,
			"level"				: 1,
			"spaceUType"		: spaceUType,
			"direction"			: (0, 0, d_avatar_inittab.datas[roleType]["spawnYaw"]),
			"position"			: spaceData.get("spawnPos", (0,0,0))
			}
			
		avatar = KBEngine.createBaseLocally('Avatar', props)
		if avatar:
			avatar.writeToDB(self._onCharacterSaved)
		
		DEBUG_MSG("Account[%i].reqCreateAvatar:%s. spaceUType=%i, spawnPos=%s.\n" % (self.id, name, avatar.cellData["spaceUType"], spaceData.get("spawnPos", (0,0,0))))
		
	def reqRemoveAvatar(self, name):
		"""
		exposed.
		客户端请求删除一个角色
		"""
		found = 0
		for key, info in self.characters.items():
			if info[0] == name:
				del self.characters[key]
				found = key
				break
		
		self.client.onRemoveAvatar(found)
		
	def _onCharacterSaved(self, success, avatar):
		"""
		新建角色写入数据库回调
		"""
		INFO_MSG('Account::_onCharacterSaved:(%i) create avatar state: %i, %s, %i' % (self.id, success, avatar.cellData["name"], avatar.databaseID))
		
		avatarinfo = TAvatarInfos()
		avatarinfo.extend([0, "", 0, 0, TAvatarData().createFromDict({"param1" : 0, "param2" :b''})])

		if success:
			info = TAvatarInfos()
			info.extend([avatar.databaseID, avatar.cellData["name"], avatar.roleType, 1, TAvatarData().createFromDict({"param1" : 1, "param2" :b'1'})])
			self.characters[avatar.databaseID] = info
			avatarinfo[0] = avatar.databaseID
			avatarinfo[1] = avatar.cellData["name"]
			avatarinfo[2] = avatar.roleType
			avatarinfo[3] = 1
			self.writeToDB()

			avatar.destroy()
		
		if not self.isDestroyed and self.client:
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
				self.lastSelCharacter = dbid
				player = KBEngine.createBaseFromDBID("Avatar", dbid, self.__onAvatarCreated)
			else:
				ERROR_MSG("Account[%i]::selectAvatarGame: not found dbid(%i)" % (self.id, dbid))
		else:
			self.giveClientTo(self.activeCharacter)
			
	def __onAvatarCreated(self, baseRef, dbid, wasActive):
		"""
		选择角色进入游戏时被调用
		"""
		if wasActive:
			ERROR_MSG("Account::__onAvatarCreated:(%i): this character is in world now!" % (self.id))
			return
		if baseRef is None:
			ERROR_MSG("Account::__onAvatarCreated:(%i): the character you wanted to created is not exist!" % (self.id))
			return
			
		avatar = KBEngine.entities.get(baseRef.id)
		if avatar is None:
			ERROR_MSG("Account::__onAvatarCreated:(%i): when character was created, it died as well!" % (self.id))
			return
		
		if self.isDestroyed:
			ERROR_MSG("Account::__onAvatarCreated:(%i): i dead, will the destroy of Avatar!" % (self.id))
			avatar.destroy()
			return
			
		info = self.characters[dbid]
		avatar.cellData["modelID"] = d_avatar_inittab.datas[info[2]]["modelID"]
		avatar.cellData["modelScale"] = d_avatar_inittab.datas[info[2]]["modelScale"]
		avatar.cellData["moveSpeed"] = d_avatar_inittab.datas[info[2]]["moveSpeed"]
		avatar.accountEntity = self
		self.activeCharacter = avatar
		self.giveClientTo(avatar)
		