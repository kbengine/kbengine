# -*- coding: utf-8 -*-
import KBEngine
import Functor
from KBEDebug import *
import d_entities
import d_spaces
import copy

CONST_WAIT_CREATE = -1

class SpaceAlloc:
	"""
	普通的场景分配器
	"""
	def __init__(self, utype):
		self._spaces = {}
		self._utype = utype
		self._pendingLogonEntities = []
		self._pendingEnterEntityMBs = []
		
	def init(self):
		"""
		virtual method.
		"""
		self.createSpace(0, {})
	
	def createSpace(self, spaceKey, context):
		"""
		"""
		context = copy.copy(context)
		spaceData = d_spaces.datas.get(self._utype)
		KBEngine.createBaseAnywhere(spaceData["entityType"], \
											{"spaceUType" : self._utype,	\
											"spaceKey" : spaceKey,	\
											"context" : context,	\
											}, \
											Functor.Functor(self.onSpaceCreatedCB, spaceKey))
											
	def onSpaceCreatedCB(self, spaceKey, space):
		"""
		一个space创建好后的回调
		"""
		DEBUG_MSG("Spaces::onSpaceCreatedCB: space %i. entityID=%i" % (self._utype, space.id))
		
	def onSpaceGetCell(self, spaceMailbox, spaceKey):
		"""
		space的cell创建好了
		"""
		DEBUG_MSG("Spaces::onSpaceGetCell: space %i. entityID=%i, spaceKey=%i" % (self._utype, spaceMailbox.id, spaceKey))
		
		if spaceKey <= 0:
			spaceKey = spaceMailbox.id

		self._spaces[spaceKey] = spaceMailbox

		pendingLogonEntities = self._pendingLogonEntities
		pendingEnterEntityMBs = self._pendingEnterEntityMBs
		self._pendingLogonEntities = []
		self._pendingEnterEntityMBs = []
		
		for e in pendingLogonEntities:
			self.loginToSpace(e)
		
		for mb, pos, dir, context in pendingEnterEntityMBs:
			self.teleportSpace(mb, pos, dir, context)
		
	def alloc(self, context):
		"""
		virtual method.
		分配一个space
		"""
		if self._spaces == {}:
			return None
		
		return list(self._spaces.values())[0]
		
	def loginToSpace(self, avatarEntity):
		"""
		virtual method.
		某个玩家请求登陆到某个space中
		"""
		space = self.alloc({"spaceKey" : avatarEntity.dbidB})
		if space is None:
			ERROR_MSG("Spaces::loginToSpace: not found space %i. login to space is failed! spaces=%s" % (self._utype, self._spaces))
			return
		
		if space == CONST_WAIT_CREATE:
			self._pendingLogonEntities.append(avatarEntity)
			DEBUG_MSG("Spaces::loginToSpace: avatarEntity=%s add pending." % avatarEntity.id)
			return
		
		DEBUG_MSG("Spaces::loginToSpace: avatarEntity=%s" % avatarEntity.id)
		space.loginToSpace(avatarEntity)

	def teleportSpace(self, entityMailbox, position, direction, context):
		"""
		virtual method.
		请求进入某个space中
		"""
		space = self.alloc(context)
		if space is None:
			ERROR_MSG("Spaces::teleportSpace: not found space %i. login to space is failed!" % self._utype)
			return
		
		if space == CONST_WAIT_CREATE:
			self._pendingEnterEntityMBs.append((entityMailbox, position, direction, context))
			DEBUG_MSG("Spaces::teleportSpace: avatarEntity=%s add pending." % entityMailbox.id)
			return
			
		DEBUG_MSG("Spaces::teleportSpace: entityMailbox=%s" % entityMailbox)
		space.teleportSpace(entityMailbox, position, direction, context)
		
class SpaceAllocCopy(SpaceAlloc):
	"""
	副本分配器
	"""
	def __init__(self, utype):
		SpaceAlloc.__init__(self, utype)

	def init(self):
		"""
		virtual method.
		"""
		pass # 副本不需要初始化创建一个
		
	def alloc(self, context):
		"""
		virtual method.
		分配一个space
		对于副本来说创建副本则将玩家的dbid作为space的key，
		任何一个人想进入到这个副本需要知道这个key。
		"""
		spaceKey = context.get("spaceKey", 0)
		space = self._spaces.get(spaceKey)
		
		assert spaceKey != 0
		
		if space is None:
			self.createSpace(spaceKey, context)
			return CONST_WAIT_CREATE
		
		return space
