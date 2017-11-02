# -*- coding: utf-8 -*-
import time, weakref, threading

from django.conf import settings

from pycommon import Machines, Define

class MachinesMgr(object):
	"""
	machines管理（缓冲）器
	"""
	instance = None
	
	def __init__(self):
		"""
		"""
		print("MachinesMgr::__init__(), USE_MACHINES_BUFFER = %s" % settings.USE_MACHINES_BUFFER)
		if self.instance is not None:
			assert False
		
		self.instance = weakref.proxy(self)
		
		self.machineInst = Machines.Machines(0, "WebConsole")
		self.interfaces_groups = {} # { machineID : [ComponentInfo, ...], ...}
		self.machines = []
		
		# 是否已经初始化过缓冲区
		# 用于当 settings.USE_MACHINES_BUFFER == True 时
		self.inited = False
		
		# 最后一次查询的时间
		# 在settings.USE_MACHINES_BUFFER == False时，用于避免外部代码在同一时间多次query all interfaces；
		# 在settings.USE_MACHINES_BUFFER == True时，用于子线程判断停止query all interfaces的时机。
		self.lastQueryTime = 0.0
	
	def __delete__(self):
		print("MachinesMgr::__delete__()")
	
	def queryAllDatas(self):
		"""
		"""
		hosts = "<broadcast>"
		if isinstance(settings.MACHINES_ADDRESS, (tuple, list)) and settings.MACHINES_ADDRESS:
			hosts = settings.MACHINES_ADDRESS
		
		self.machineInst.queryAllInterfaces(hosts, 0, settings.MACHINES_QUERY_WAIT_TIME)
		
		# 虽然处于多线程中，但由于是类似于指针直接赋值
		# 所以即使异步了，主线程取的值也不会存在错乱的问题
		self.interfaces_groups = self.machineInst.interfaces_groups
		self.machines = self.machineInst.machines
		self.machineInst.reset()
	
	def startThread(self):
		"""
		开启线程
		"""
		self.thread = threading.Thread(None, self.threadRun, "MachinesDetecter")
		self.thread.start()
	
	def threadRun(self):
		"""
		子线程定时获取数据
		"""
		use_buffer = settings.USE_MACHINES_BUFFER
		while use_buffer and settings.USE_MACHINES_BUFFER and time.time() - self.lastQueryTime < settings.STOP_BUFFER_TIME:  # 动态更新需要
			self.queryAllDatas()
			self.inited = True
			
			# 每更新一次后休眠一段时间
			time.sleep(settings.MACHINES_BUFFER_FLUSH_TIME)
		
		print("MachinesMgr::threadRun(), over! old %s, new %s, last query: %s" % (use_buffer, settings.USE_MACHINES_BUFFER, time.time() - self.lastQueryTime))
		self.thread = None
		self.inited = False
		self.interfaces_groups = []
		self.machines = []

	def checkAndQueryInterfaces(self):
		"""
		"""
		if settings.USE_MACHINES_BUFFER:
			self.lastQueryTime = time.time()
			if not self.inited:
				self.startThread()
				while not self.inited:
					time.sleep(0.5)
		else:
			if time.time() - self.lastQueryTime >= 1.0:
				self.queryAllDatas()
				self.lastQueryTime = time.time()

	def filterComponentsForUID(self, uid, interfaces_groups):
		"""
		"""
		if uid <= 0:
			return interfaces_groups

		result = {} # { machineID : [ComponentInfo, ...], ...}
		for k, v in interfaces_groups.items():
			result[k] = [e for e in v if e.uid == uid]
		return result

	def queryAllInterfaces(self, uid, user):
		"""
		"""
		self.checkAndQueryInterfaces()
		ig = self.interfaces_groups
		return self.filterComponentsForUID(uid, ig)

	
	def queryMachines(self):
		"""
		获取所有machines数据
		"""
		self.checkAndQueryInterfaces()
		return self.machines

	def hasMachine(self, machineHost):
		"""
		判断是否存在特定的machine(机器)
		"""
		self.checkAndQueryInterfaces()

		# 先引用再使用，以避免此过程中self.machines引用被修改
		ms = self.machines
		for info in ms:
			if info.intaddr == machineHost:
				return True
		return False
		
	def makeGUS(self, componentType):
		"""
		生成一个相对唯一的gus（非全局唯一）
		"""
		return self.machineInst.makeGUS(componentType)
	
	def makeCID(self, componentType):
		"""
		生成相对唯一的cid（非全局唯一）
		"""
		return self.machineInst.makeCID(componentType)

# 全局变量
machinesmgr = MachinesMgr()

