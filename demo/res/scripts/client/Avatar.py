# -*- coding: utf-8 -*-
import KBEngine
import KBExtend
from KBEDebug import *
from GameObject import GameObject

class Avatar(GameObject):
	def __init__(self):
		GameObject.__init__(self)

	def onEnterSpace(self):
		"""
		KBEngine method.
		这个entity进入了一个新的space
		"""
		DEBUG_MSG("%s[%i]." % (self.__class__.__name__, self.id))

	def onLeaveSpace(self):
		"""
		KBEngine method.
		这个entity将要离开当前space
		"""
		DEBUG_MSG("%s[%i]." % (self.__class__.__name__, self.id))
		
	def onBecomePlayer( self ):
		"""
		KBEngine method.
		当这个entity被引擎定义为角色时被调用
		"""
		DEBUG_MSG("%s[%i]." % (self.__class__.__name__, self.id))