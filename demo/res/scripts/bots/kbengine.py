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

def onStart():
	"""
	KBEngine method.
	在onInitialize调用之后， 准备开始游戏时引擎调用这个接口.
	"""
	pass
	
def onFinish():
	"""
	KBEngine method.
	客户端将要关闭时， 引擎调用这个接口
	可以在此做一些游戏资源清理工作
	"""
	pass
	
def onKeyPressed(down, key, mods):
	"""
	KBEngine method.
	某个键按下时的回调
	@param mods	: 修饰键是否按下 修饰键包括:Shift, Ctrl, Alt
	"""
	pass

def onMousePressedInWorld(dx, dy, dz):
	"""
	KBEngine method.
	鼠标在3D场景任意位置按下时的回调
	注意：如果在gui上按下鼠标， 本处不会触发回调， 这是由于我们客户端
	的gui等等部分不由脚本处理而是由用户在kbe之外扩展， 因此无需获得相关事件通知。
	"""
	player = KBEngine.player()
	if player:
		player.seek((dx / 10.0, dy / 10.0, dz / 10.0), player.moveSpeed * 0.1, 1, 1, None)