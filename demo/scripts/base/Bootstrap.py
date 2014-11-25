# -*- coding: utf-8 -*-
import KBEngine
from KBEDebug import *

class BootObject:
	"""
	引导对象：开发者可以扩展出不同的引导对象添加到引导程序进行引导
	"""
	def __init__(self):
		pass
	
	def name(self):
		"""
		virtual method.
		"""
		return "undefined"
		
	def priority(self):
		"""
		virtual method.
		获得执行优先级
		返回值越高优先级越高
		"""
		return 0
		
	def onBootStart(self, bootstrapIdx):
		"""
		virtual method.
		被引导时触发
		"""
		pass

	def onKillBoot(self, bootstrapIdx):
		"""
		virtual method.
		卸载时触发
		"""
		pass

	def readyForLogin(self, bootstrapIdx):
		"""
		virtual method.
		是否引导完毕。
		1.0代表100%完成，loginapp允许登录
		"""
		return 1.0
		
class Bootstrap:
	"""
	引导程序：服务端进程起来后开始引导
	"""
	bootstrapIdx = 0
	bootObjects = {}

	@staticmethod
	def add(bootObject):
		objs = Bootstrap.bootObjects.get(bootObject.priority(), None)
		if objs is None:
			objs = []
			Bootstrap.bootObjects[bootObject.priority()] = objs
			
		if bootObject not in objs:
			objs.append(bootObject)
		
	@staticmethod
	def start(bootstrapIdx):
		Bootstrap.bootstrapIdx = bootstrapIdx
		
		pris = list(Bootstrap.bootObjects.keys())
		sorted(pris, reverse = True)
		
		for pri in pris:
			for bo in Bootstrap.bootObjects[pri]:
				DEBUG_MSG("Bootstrap::start(): booting %s..." % bo.name())
				bo.onBootStart(bootstrapIdx)

	@staticmethod
	def readyForLogin(bootstrapIdx):
		"""
		virtual method.
		是否引导完毕，loginapp允许登录
		"""
		pris = list(Bootstrap.bootObjects.keys())
		
		if len(pris) == 0:
			return 1.0

		sorted(pris, reverse = True)
		
		ret = 0.0
		count = 0
		
		for pri in pris:
			for bo in Bootstrap.bootObjects[pri]:
				ret += bo.readyForLogin(bootstrapIdx)
				count += 1
				
		return (ret / count) + 0.001
		
	@staticmethod
	def end(state):
		pris = list(Bootstrap.bootObjects.keys())
		sorted(pris)
		
		for pri in pris:
			for bo in Bootstrap.bootObjects[pri]:
				DEBUG_MSG("Bootstrap::end(): kill %s..." % bo.name())
				bo.onKillBoot(Bootstrap.bootstrapIdx)