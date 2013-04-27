# -*- coding: utf-8 -*-
import KBEngine
from KBEDebug import *

class GameObject(KBEngine.Base):
	def __init__(self):
		KBEngine.Base.__init__(self)

	def getScriptName(self):
		return self.__class__.__name__
		
	def onTimer(self, tid, userArg):
		"""
		KBEngine method.
		引擎回调timer触发
		"""
		#DEBUG_MSG("%s::onTimer: %i, tid:%i, arg:%i" % (self.getScriptName(), self.id, tid, userArg))
		if self.isDestroyed:
			self.delTimer(tid)
			return
			
		self._timermap[userArg](self, tid, userArg)

	def destroySelf(self):
		"""
		virtual method
		"""
		if self.cell is not None:
			# 销毁cell实体
			self.destroyCellEntity()
			return
			
		# 销毁base
		self.destroy()

	def onGetCell(self):
		"""
		KBEngine method.
		entity的cell部分实体被创建成功
		"""
		DEBUG_MSG("%s::onGetCell: %i" % (self.getScriptName(), self.id))
		
	def onLoseCell(self):
		"""
		KBEngine method.
		entity的cell部分实体丢失
		"""
		DEBUG_MSG("%s::onLoseCell: %i" % (self.getScriptName(), self.id))
		self.destroySelf()

	def onRestore(self):
		"""
		KBEngine method.
		entity的cell部分实体被恢复成功
		"""
		DEBUG_MSG("%s::onRestore: %s" % (self.getScriptName(), self.cell))
		
GameObject._timermap = {}
