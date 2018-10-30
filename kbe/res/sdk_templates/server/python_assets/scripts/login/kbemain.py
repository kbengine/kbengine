# -*- coding: utf-8 -*-
import os
import KBEngine
from KBEDebug import *

"""
loginapp进程主要处理KBEngine服务端登陆、创建账号等工作。
目前脚本支持几种功能:
1: 注册账号检查
2：登陆检查
3：自定义socket回调，参考interface中Poller实现
"""


def onLoginAppReady():
	"""
	KBEngine method.
	loginapp已经准备好了
	"""
	INFO_MSG('onLoginAppReady: bootstrapGroupIndex=%s, bootstrapGlobalIndex=%s' % \
	 (os.getenv("KBE_BOOTIDX_GROUP"), os.getenv("KBE_BOOTIDX_GLOBAL")))

	#KBEngine.addTimer(0.01, 1.0, onTick)

def onTick(timerID):
	"""
	"""
	INFO_MSG('onTick()')

def onLoginAppShutDown():
	"""
	KBEngine method.
	这个loginapp被关闭前的回调函数
	"""
	INFO_MSG('onLoginAppShutDown()')

def onRequestLogin(loginName, password, clientType, datas):
	"""
	KBEngine method.
	账号请求登陆时回调
	此处还可以对登陆进行排队，将排队信息存放于datas
	"""
	INFO_MSG('onRequestLogin() loginName=%s, clientType=%s' % (loginName, clientType))

	errorno = KBEngine.SERVER_SUCCESS
	
	if len(loginName) > 64:
		errorno = KBEngine.SERVER_ERR_NAME

	if len(password) > 64:
		errorno = KBEngine.SERVER_ERR_PASSWORD

	return (errorno, loginName, password, clientType, datas)

def onLoginCallbackFromDB(loginName, accountName, errorno, datas):
	"""
	KBEngine method.
	账号请求登陆后db验证回调
	loginName：登录名既登录时客户端输入的名称。
	accountName: 账号名则是dbmgr查询得到的名称。
	errorno: KBEngine.SERVER_ERR_*
	这个机制用于一个账号多名称系统或者多个第三方账号系统登入服务器。
	客户端得到baseapp地址的同时也会返回这个账号名称，客户端登陆baseapp应该使用这个账号名称登陆
	"""
	INFO_MSG('onLoginCallbackFromDB() loginName=%s, accountName=%s, errorno=%s' % (loginName, accountName, errorno))
	
def onRequestCreateAccount(accountName, password, datas):
	"""
	KBEngine method.
	请求账号创建时回调
	"""
	INFO_MSG('onRequestCreateAccount() %s' % (accountName))

	errorno = KBEngine.SERVER_SUCCESS
	
	if len(accountName) > 64:
		errorno = KBEngine.SERVER_ERR_NAME

	if len(password) > 64:
		errorno = KBEngine.SERVER_ERR_PASSWORD
		
	return (errorno, accountName, password, datas)

def onCreateAccountCallbackFromDB(accountName, errorno, datas):
	"""
	KBEngine method.
	账号请求注册后db验证回调
	errorno: KBEngine.SERVER_ERR_*
	"""
	INFO_MSG('onCreateAccountCallbackFromDB() accountName=%s, errorno=%s' % (accountName, errorno))
