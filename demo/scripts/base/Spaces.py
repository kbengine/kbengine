# -*- coding: utf-8 -*-
import KBEngine
import Functor
import d_spaces
import SCDefine
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

# 添加到引导程序，以便在服务端启动后能够创建这个实体
Bootstrap.add(BootSpaces())

class Spaces(KBEngine.Base, GameObject):
	"""
	这是一个脚本层封装的空间管理器
	KBEngine的space是一个抽象空间的概念，一个空间可以被脚本层视为游戏场景、游戏房间、甚至是一个宇宙。
	"""
	def __init__(self):
		KBEngine.Base.__init__(self)
		GameObject.__init__(self)
		
		# 初始化空间分配器
		self.initAlloc()
		
		# 向全局共享数据中注册这个管理器的mailbox以便在所有逻辑进程中可以方便的访问
		KBEngine.globalData["Spaces"] = self
	
	def initAlloc(self):
		# 注册一个定时器，在这个定时器中我们每个周期都创建出一些NPC，直到创建完所有
		self._spaceAllocs = {}
		self.addTimer(3, 1, SCDefine.TIMER_TYPE_CREATE_SPACES)
		
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
		defined method.
		某个玩家请求登陆到某个space中
		"""
		self._spaceAllocs[spaceUType].loginToSpace(avatarEntity, context)
	
	def logoutSpace(self, avatarID, spaceID):
		"""
		defined method.
		某个玩家请求登出这个space
		"""
		for spaceAlloc in self._spaceAllocs.values():
			space = spaceAlloc.getSpaces().get(spaceID)
			if space:
				space.logoutSpace(avatarID)
				
	def teleportSpace(self, entityMailbox, spaceUType, position, direction, context):
		"""
		defined method.
		请求进入某个space中
		"""
		self._spaceAllocs[spaceUType].teleportSpace(entityMailbox, position, direction, context)

	def onSpaceLoseCell(self, spaceUType, spaceKey):
		"""
		defined method.
		space的cell创建好了
		"""
		self._spaceAllocs[spaceUType].onSpaceLoseCell(spaceKey)
		
	def onSpaceGetCell(self, spaceUType, spaceMailbox, spaceKey):
		"""
		defined method.
		space的cell创建好了
		"""
		self._spaceAllocs[spaceUType].onSpaceGetCell(spaceMailbox, spaceKey)

Spaces._timermap = {}
Spaces._timermap.update(GameObject._timermap)
Spaces._timermap[SCDefine.TIMER_TYPE_CREATE_SPACES] = Spaces.createSpaceOnTimer
