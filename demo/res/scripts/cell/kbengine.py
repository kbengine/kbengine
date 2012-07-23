# -*- coding: utf-8 -*-
import KBEngine
from KBEDebug import *

def onInit(isReload):
	"""
	KBEngine method.
	当引擎启动后初始化完所有的脚本后这个接口被调用
	"""
	DEBUG_MSG('所有的脚本初始化完毕。游戏初始化...')
	
def onGlobalData(key, value):
	DEBUG_MSG('onGlobalData: %s' % key)
	
def onGlobalDataDel(key):
	DEBUG_MSG('onDelGlobalData: %s' % key)
	
def onSpaceData( spaceID, entryID, key, value ):
	pass
	
def onAllSpaceGeometryLoaded( spaceID, isBootstrap, mapping ):
	pass