# -*- coding: utf-8 -*-
import os
import KBEngine
from KBEDebug import *

"""
interfaces进程主要处理KBEngine服务端与第三方平台的接入接出工作。
目前支持几种功能:
1: 注册账号
"""


def onLoginAppReady():
	"""
	KBEngine method.
	interfaces已经准备好了
	"""
	INFO_MSG('onLoginAppReady: bootstrapGroupIndex=%s, bootstrapGlobalIndex=%s' % \
	 (os.getenv("KBE_BOOTIDX_GROUP"), os.getenv("KBE_BOOTIDX_GLOBAL")))

	KBEngine.addTimer(0.01, 1.0, onTick)

def onTick(timerID):
	"""
	"""
	INFO_MSG('onTick()')
	pass

def onLoginAppShutDown():
	"""
	KBEngine method.
	这个interfaces被关闭前的回调函数
	"""
	INFO_MSG('onLoginAppShutDown()')

def checkLogin(loginName, password, clientType, datas):
	"""
	KBEngine method.
	这个interfaces被关闭前的回调函数
	"""
	INFO_MSG('checkLogin() %s' % (accountName))

def checkCreateAccount(accountName, accountPassword):
	"""
	KBEngine method.
	这个interfaces被关闭前的回调函数
	"""
	INFO_MSG('checkLogin() %s' % (accountName))
