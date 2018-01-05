# -*- coding: utf-8 -*-
import KBEngine
from KBEDebug import *

def onInit(isReload):
	"""
	KBEngine method.
	当引擎启动后初始化完所有的脚本后这个接口被调用
	@param isReload: 是否是被重写加载脚本后触发的
	@type isReload: bool
	"""
	DEBUG_MSG('onInit::isReload:%s' % isReload)
	
def onFinish():
	"""
	KBEngine method.
	引擎将要关闭时， 引擎调用这个接口
	可以在此做一些游戏资源清理工作
	"""
	pass
