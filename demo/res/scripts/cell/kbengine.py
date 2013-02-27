# -*- coding: utf-8 -*-
import KBEngine
from KBEDebug import *
import dialogmgr
import skills

def onInit(isReload):
	"""
	KBEngine method.
	当引擎启动后初始化完所有的脚本后这个接口被调用
	"""
	DEBUG_MSG('onInit::isReload:%s' % isReload)
	dialogmgr.onInit()
	skills.onInit()
	
def onGlobalData(key, value):
	DEBUG_MSG('onGlobalData: %s' % key)
	
def onGlobalDataDel(key):
	DEBUG_MSG('onDelGlobalData: %s' % key)

def onCellAppData(key, value):
	DEBUG_MSG('onCellAppData: %s' % key)
	
def onCellAppDataDel(key):
	DEBUG_MSG('onCellAppDataDel: %s' % key)
	
def onSpaceData( spaceID, entryID, key, value ):
	pass
	
def onAllSpaceGeometryLoaded( spaceID, isBootstrap, mapping ):
	pass