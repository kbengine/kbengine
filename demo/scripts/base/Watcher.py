# -*- coding: utf-8 -*-
import KBEngine
from KBEDebug import *
from Bootstrap import Bootstrap, BootObject

class BootWatcher(BootObject):
	"""
	引导对象：开发者可以扩展出不同的引导对象添加到引导程序进行引导
	"""
	def __init__(self):
		BootObject.__init__(self)
	
	def name(self):
		"""
		virtual method.
		"""
		return "Watcher"
		
	def onBootStart(self, bootstrapIdx):
		"""
		virtual method.
		被引导时触发
		"""
		KBEngine.addWatcher("players", "UINT32", countPlayers)

Bootstrap.add(BootWatcher())

def countPlayers():
	"""
	"""
	i = 0
	for e in KBEngine.entities.values():
		if e.__class__.__name__ == "Avatar":
			i += 1
			
	return i
	


