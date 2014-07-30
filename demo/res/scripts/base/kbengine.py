# -*- coding: utf-8 -*-
import KBEngine
import d_spaces
from KBEDebug import *

def countPlayers():
	"""
	"""
	i = 0
	for e in KBEngine.entities.values():
		if e.__class__.__name__ == "Avatar":
			i += 1
			
	return i
	
def onBaseAppReady(bootstrapIdx):
	"""
	KBEngine method.
	baseapp已经准备好了
	@param bootstrapIdx: 当前baseapp的启动顺序
	@type bootstrapIdx: INT32
	"""
	INFO_MSG('onBaseAppReady: bootstrapIdx=%s' % bootstrapIdx)
	KBEngine.addWatcher("players", "UINT32", countPlayers)
	
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
	INFO_MSG('onBaseAppShutDown: state=%i' % state)

def readlyForLogin(bootstrapIdx):
	"""
	KBEngine method.
	如果返回值大于等于1.0则初始化全部完成, 否则返回准备的进度值0.0~1.0。
	在此可以确保脚本层全部初始化完成之后才开放登录。
	@param bootstrapIdx: 当前baseapp的启动顺序
	@type bootstrapIdx: INT32
	"""
	if bootstrapIdx != 1:
		INFO_MSG('initProgress: completed!')
		return True
		
	spacesEntity = KBEngine.globalData["SpaceMgr"]
	
	tmpDatas = list(d_spaces.datas.keys())
	count = 0
	total = len(tmpDatas)
	
	for utype in tmpDatas:
		spaceAlloc = spacesEntity.getSpaceAllocs()[utype]
		if spaceAlloc.__class__.__name__ != "SpaceAllocDuplicate":
			if len(spaceAlloc.getSpaces()) > 0:
				count += 1
		else:
			count += 1
	
	if count < total:
		v = float(count) / total
		# INFO_MSG('initProgress: %f' % v)
		return v;
	
	INFO_MSG('initProgress: completed!')
	return 1.0
	
def onInit(isReload):
	"""
	KBEngine method.
	当引擎启动后初始化完所有的脚本后这个接口被调用
	@param isReload: 是否是被重写加载脚本后触发的
	@type isReload: bool
	"""
	INFO_MSG('onInit::isReload:%s' % isReload)

def onFini():
	"""
	KBEngine method.
	引擎正式关闭
	"""
	INFO_MSG('onFini()')
	
def onCellAppDeath(addr):
	"""
	KBEngine method.
	某个cellapp死亡
	"""
	WARNING_MSG('onCellAppDeath: %s' % (str(addr)))
	
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

def onBaseAppData(key, value):
	"""
	KBEngine method.
	baseAppData有改变
	"""
	DEBUG_MSG('onBaseAppData: %s' % key)
	
def onBaseAppDataDel(key):
	"""
	KBEngine method.
	baseAppData有删除
	"""
	DEBUG_MSG('onBaseAppDataDel: %s' % key)

def onLoseChargeCB(ordersID, dbid, success, datas):
	"""
	KBEngine method.
	有一个不明订单被处理， 可能是超时导致记录被billing
	清除， 而又收到第三方充值的处理回调
	"""
	DEBUG_MSG('onLoseChargeCB: ordersID=%s, dbid=%i, success=%i, datas=%s' % \
							(ordersID, dbid, success, datas))


