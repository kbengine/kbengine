# -*- coding: utf-8 -*-
import KBEngine
from KBEDebug import *

def countPlayers():
	"""
	KBEngine.addWatcher("players", "UINT32", countPlayers)
	上面代码将这个函数添加到监视器中，可以从GUIConsole等工具中实时监视到函数返回值
	"""
	i = 0
	for e in KBEngine.entities.values():
		if e.__class__.__name__ == "Avatar":
			i += 1

	return i
	


def setup():
	KBEngine.addWatcher("players", "UINT32", countPlayers)