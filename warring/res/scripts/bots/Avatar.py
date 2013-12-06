# -*- coding: utf-8 -*-
import KBEngine
import KBExtra
from KBEDebug import *
from interfaces.GameObject import GameObject
from interfaces.Dialog import Dialog
from interfaces.Teleport import Teleport
from interfaces.State import State
from interfaces.Flags import Flags
from interfaces.Combat import Combat
from interfaces.Spell import Spell
from interfaces.SkillBox import SkillBox

class Avatar(GameObject,
			Flags,
			State,
			Combat, 
			Spell, 
			Dialog,
			Teleport):
	def __init__(self):
		GameObject.__init__(self)
		Flags.__init__(self) 
		State.__init__(self) 
		SkillBox.__init__(self) 
		Combat.__init__(self) 
		Spell.__init__(self) 
		Dialog.__init__(self)
		Teleport.__init__(self)
		
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
		
	def onJump(self):
		"""
		defined method.
		玩家跳跃
		"""
		pass
		
		
class PlayerAvatar(Avatar):
	def __init__(self):
		Avatar.__init__()

