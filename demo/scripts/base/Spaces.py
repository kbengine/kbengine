# -*- coding: utf-8 -*-
import KBEngine
import Functor
import d_spaces
import wtimer
import Watcher
from Bootstrap import Bootstrap, BootObject
from KBEDebug import *
from SpaceAlloc import *
from interfaces.GameObject import GameObject

class BootSpaces(BootObject):
	"""
	引导对象：开发者可以扩展出不同的引导对象添加到引导程序进行引导
	"""
	def __init__(self):
		BootObject.__init__(self)

	def name(self):
		"""
		virtual method.
		"""
		return "Spaces"
		
	def priority(self):
		"""
		virtual method.
		获得执行优先级
		返回值越高优先级越高
		"""
		return 999
		
	def onBootStart(self, bootstrapIdx):
		"""
		virtual method.
		被引导时触发
		"""
		if bootstrapIdx == 1:
			# 创建spacemanager
			KBEngine.createBaseLocally( "Spaces", {} )

	def onKillBoot(self, bootstrapIdx):
		"""
		virtual method.
		卸载时触发
		"""
		pass

	def readyForLogin(self, bootstrapIdx):
		"""
		virtual method.
		是否引导完毕。
		1.0代表100%完成，loginapp允许登录
		"""
		if bootstrapIdx != 1:
			return 1.0

		spacesEntity = KBEngine.globalData["Spaces"]
		
		tmpDatas = list(d_spaces.datas.keys())
		count = 0
		total = len(tmpDatas)
		
		for utype in tmpDatas:
			spaceAlloc = spacesEntity.getSpaceAllocs()[utype]
			if spaceAlloc.__class__.__name__ != "SpaceAllocDuplicate":
				if len(spaceAlloc.getSpaces()) > 0:
					count += 1
			else:
				count += 1
		
		if count < total:
			v = float(count) / total
			# INFO_MSG('initProgress: %f' % v)
			return v;

		return 1.0
		
Bootstrap.add(BootSpaces())

class Spaces(KBEngine.Base, GameObject):
	def __init__(self):
		KBEngine.Base.__init__(self)
		GameObject.__init__(self)
		
		self._spaceAllocs = {}
		self.addTimer(3, 1, wtimer.TIMER_TYPE_CREATE_SPACES)
		self.initAlloc()
		KBEngine.globalData["Spaces"] = self
	
	def initAlloc(self):
		self._tmpDatas = list(d_spaces.datas.keys())
		for utype in self._tmpDatas:
			spaceData = d_spaces.datas.get(utype)
			if spaceData["entityType"] == "SpaceDuplicate":
				self._spaceAllocs[utype] = SpaceAllocDuplicate(utype)
			else:
				self._spaceAllocs[utype] = SpaceAlloc(utype)
	
	def getSpaceAllocs(self):
		return self._spaceAllocs
		
	def createSpaceOnTimer(self, tid, tno):
		"""
		创建space
		"""
		if len(self._tmpDatas) > 0:
			spaceUType = self._tmpDatas.pop(0)
			self._spaceAllocs[spaceUType].init()
			
		if len(self._tmpDatas) <= 0:
			del self._tmpDatas
			self.delTimer(tid)
			
	def loginToSpace(self, avatarEntity, spaceUType, context):
		"""
		define method.
		某个玩家请求登陆到某个space中
		"""
		self._spaceAllocs[spaceUType].loginToSpace(avatarEntity, context)
	
	def logoutSpace(self, avatarID, spaceID):
		"""
		define method.
		某个玩家请求登出这个space
		"""
		for spaceAlloc in self._spaceAllocs.values():
			space = spaceAlloc.getSpaces().get(spaceID)
			if space:
				space.logoutSpace(avatarID)
				
	def teleportSpace(self, entityMailbox, spaceUType, position, direction, context):
		"""
		define method.
		请求进入某个space中
		"""
		self._spaceAllocs[spaceUType].teleportSpace(entityMailbox, position, direction, context)

	def onSpaceLoseCell(self, spaceUType, spaceKey):
		"""
		define method.
		space的cell创建好了
		"""
		self._spaceAllocs[spaceUType].onSpaceLoseCell(spaceKey)
		
	def onSpaceGetCell(self, spaceUType, spaceMailbox, spaceKey):
		"""
		define method.
		space的cell创建好了
		"""
		self._spaceAllocs[spaceUType].onSpaceGetCell(spaceMailbox, spaceKey)

Spaces._timermap = {}
Spaces._timermap.update(GameObject._timermap)
Spaces._timermap[wtimer.TIMER_TYPE_CREATE_SPACES] = Spaces.createSpaceOnTimer
