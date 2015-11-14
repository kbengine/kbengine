# -*- coding: utf-8 -*-
import os
import KBEngine
from KBEDebug import *

"""
interfaces进程主要处理KBEngine服务端与第三方平台的接入接出工作。
目前支持几种功能:
1: 注册账号
"""


def onDBMgrReady():
	"""
	KBEngine method.
	dbmgr已经准备好了
	"""
	INFO_MSG('onDBMgrReady: bootstrapGroupIndex=%s, bootstrapGlobalIndex=%s' % \
	 (os.getenv("KBE_BOOTIDX_GROUP"), os.getenv("KBE_BOOTIDX_GLOBAL")))

	KBEngine.addTimer(0.01, 1.0, onTick)

def onTick(timerID):
	"""
	"""
	#INFO_MSG('onTick()')
	pass

def onDBMgrShutDown():
	"""
	KBEngine method.
	这个dbmgr被关闭前的回调函数
	"""
	INFO_MSG('onDBMgrShutDown()')

