# -*- coding(self): utf-8 -*-
class KBEngine:
	def __init__(self):
		Entity = None
		Base= None
		Proxy = None
		GlobalDataClient = None
		globalData = None
		baseAppData = None
		component='baseapp'
		LOG_ON_REJECT=0
		LOG_ON_ACCEPT=1
		NEXT_ONLY=2
		LOG_ON_WAIT_FOR_DESTROY=2
		LOG_TYPE_NORMAL=21000
		LOG_TYPE_INFO=21001
		LOG_TYPE_ERR=21002
		LOG_TYPE_DBG=21003
		LOG_TYPE_WAR=21004
	def Blob(self):
		pass
	def addWatcher(self):
		pass
	def address(self):
		pass
	def charge(self):
		pass
	def createBase(self):
		pass
	def createBaseAnywhere(self):
		pass
	def createBaseAnywhereFromDBID(self):
		pass
	def createBaseFromDBID(self):
		pass
	def createBaseLocally(self):
		pass
	def createEntity(self):
		pass
	def delWatcher(self):
		pass
	def deleteBaseByDBID(self):
		pass
	def deregisterFileDescriptor(self):
		pass
	def deregisterWriteFileDescriptor(self):
		pass
	def entities(self):
		pass
	def executeRawDatabaseCommand(self):
		pass
	def genUUID64(self):
		pass
	def getResFullPath(self):
		pass
	def getWatcher(self):
		pass
	def getWatcherDir(self):
		pass
	def hasRes(self):
		pass
	def isShuttingDown(self):
		pass
	def listPathRes(self):
		pass
	def matchPath(self):
		pass
	def open(self):
		pass
	def publish(self):
		pass
	def quantumPassedPercent(self):
		pass
	def registerFileDescriptor(self):
		pass
	def registerWriteFileDescriptor(self):
		pass
	def reloadScript(self):
		pass
	def scriptLogType(self):
		pass
	def time(self):
		pass