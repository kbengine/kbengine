# -*- coding: utf-8 -*-
import KBEngine
from KBEDebug import *

def onBaseAppReady(bootstrapIdx):
	"""
	KBEngine method.
	baseapp已经准备好了
	@param isBootstrap: 是否是第一个baseapp启动
	@type isBootstrap: bool
	"""
	DEBUG_MSG('onBaseAppReady: bootstrapIdx=%s' % bootstrapIdx)
	if bootstrapIdx == 1:
		# 创建spacemanager
		KBEngine.createBaseLocally( "Spaces", {} )
		
def onBaseAppShutDown(state):
	"""
	KBEngine method.
	这个baseapp被关闭前的回调函数
	@param state:  0 : 在断开所有客户端之前
						 1 : 在将所有entity写入数据库之前
						 2 : 所有entity被写入数据库之后
	@type state: int					 
	"""
	DEBUG_MSG('onBaseAppShutDown: state=%i' % state)
		
def onInit(isReload):
	"""
	KBEngine method.
	当引擎启动后初始化完所有的脚本后这个接口被调用
	@param isReload: 是否是被重写加载脚本后触发的
	@type isReload: bool
	"""
	DEBUG_MSG('onInit::isReload:%s' % isReload)

def onFini():
	"""
	KBEngine method.
	引擎正式关闭
	"""
	pass
	
def onCellAppDeath(addr):
	"""
	KBEngine method.
	某个cellapp死亡
	"""
	pass
	
def onGlobalData(key, value):
	"""
	KBEngine method.
	globalData有改变
	"""
	DEBUG_MSG('onGlobalData: %s' % key)
	
def onGlobalDataDel(key):
	"""
	KBEngine method.
	globalData有删除
	"""
	DEBUG_MSG('onDelGlobalData: %s' % key)
	
def onGlobalBases(key, value):
	"""
	KBEngine method.
	globalBases有改变
	"""
	DEBUG_MSG('onGlobalBases: %s' % key)
	
def onGlobalBasesDel(key):
	"""
	KBEngine method.
	globalBases有删除
	"""
	DEBUG_MSG('onGlobalBasesDel: %s' % key)

def onLoseChargeCB(ordersID, dbid, success, datas):
	"""
	KBEngine method.
	有一个不明订单被处理， 可能是超时导致记录被billing
	清除， 而又收到第三方充值的处理回调
	"""
	DEBUG_MSG('onLoseChargeCB: ordersID=%s, dbid=%i, success=%i, datas=%s' % \
							(ordersID, dbid, success, datas))


