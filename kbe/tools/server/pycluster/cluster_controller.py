# -*- coding: utf-8 -*-

import socket, time
import sys
import struct
import traceback
import select
import getpass 
import time
import os

# FLOAT；单位：秒
# 每次查询Machine时等待响应的最长时间
MACHINES_QUERY_WAIT_TIME = 1.0

# FLOAT；单位：秒
# 每次查询Machine失败重试次数
MACHINES_QUERY_ATTEMPT_COUNT = 1

# MACHINES地址配置
# 当此参数不为空时，则由原来的广播探测改为固定地址探测，
# WebConsole仅会以此配置的地址进行探测，
# 当服务器存在跨网段的情况时，此方案犹为有用。
# 例子：
# MACHINES_ADDRESS = ["192.168.0.1", "10.0.0.1", "172.16.0.1"]
MACHINES_ADDRESS = []

def initRootPath():
	"""
	初始化root目录，以加载其它的脚本
	"""
	appdir = os.path.dirname( os.path.abspath( __file__ ) )
	parentDir = os.path.dirname( appdir )
	if parentDir not in sys.path:
		sys.path.append( parentDir )

initRootPath()
from pycommon.Define import *
from pycommon.LoggerWatcher import LoggerWatcher
from pycommon.Machines import Machines, ComponentInfo



# No shutdown
NO_SHUTDOWN_COMPONENTS = (
	"interfaces",
	"watcher",
	"bots",
	"logger",
	"console",
	"machine",
	"client",
)

if len(MACHINES_ADDRESS) == 0:
	MACHINES_ADDRESS = "<broadcast>"

class ClusterControllerHandler( Machines ):
	def __init__(self, uid):
		Machines.__init__(self, uid)
		
	def do(self):
		pass

class ClusterConsoleHandler(ClusterControllerHandler):
	def __init__(self, uid, consoleType):
		ClusterControllerHandler.__init__(self, uid)
		self.consoleType = consoleType
		
	def do(self):
		self.interfaces_groups = {}
		print("finding(" + self.consoleType  + ")...")
		self.queryAllInterfaces(MACHINES_ADDRESS, MACHINES_QUERY_ATTEMPT_COUNT, MACHINES_QUERY_WAIT_TIME)
		
		for machineID in self.interfaces_groups:
			infos = self.interfaces_groups.get(machineID, [])
			if len(infos) > 0:
				info = infos.pop(0)

			for info in infos:
				if info.componentName + str(info.groupOrderID) == self.consoleType or \
					info.componentName + str("%02d" %(info.groupOrderID)) == self.consoleType:
					os.system('telnet %s %i' % (info.intaddr, info.extradata3))
					return
		
		print("not found " + self.consoleType  + "!")
		
class ClusterQueryHandler(ClusterControllerHandler):
	def __init__(self, uid):
		ClusterControllerHandler.__init__(self, uid)
		
	def do(self):
		self.interfaces_groups = {}
		self.queryAllInterfaces(MACHINES_ADDRESS, MACHINES_QUERY_ATTEMPT_COUNT, MACHINES_QUERY_WAIT_TIME)
		
		numBases = 0
		numEntities = 0
		numClients = 0
		numProxices = 0
		numCells = 0
		numComponent = 0
		
		for machineID in self.interfaces_groups:
			infos = self.interfaces_groups.get(machineID, [])
			print('-----------------------------------------------------')
			if len(infos) > 0:
				info = infos.pop(0)
				print("[%s: %%CPU:%.2f, %%MEM:%.2f, %%pCPU:%.2f, pMem:%.2fm, totalMem=%.2fm/%.2fm, addr=%s]" % \
					(info.componentName, info.cpu, info.mem, info.extradata2 / 100.0, info.usedmem / 1024.0 / 1024.0, 
					 info.extradata1 / 1024.0 / 1024.0, info.extradata / 1024.0 / 1024.0, info.intaddr))
			
			numComponent += len(infos)
			
			print("      proc\t\tcid\t\tuid\tpid\tgid\t%CPU\t%MEM\tusedMem\textra1\t\textra2\t\textra3")
			for info in infos:
				if info.componentType == BASEAPP_TYPE:
					print("|-%12s\t%i\t%i\t%i\t%i\t%.2f\t%.2f\t%.2fm\tbases=%i\t\tclients=%i\tproxices=%i" % \
					((info.componentName + ("%.2i" % info.groupOrderID)), info.componentID, info.uid, info.pid, info.globalOrderID, 
					 info.cpu, info.mem, info.usedmem / 1024.0 / 1024.0, info.extradata, info.extradata1, info.extradata2))

					numBases += info.extradata
					numClients += info.extradata1
					numProxices += info.extradata2
		
				elif info.componentType == CELLAPP_TYPE:
					print("|-%12s\t%i\t%i\t%i\t%i\t%.2f\t%.2f\t%.2fm\tentities=%i\tcells=%i\t\t%i" % \
					((info.componentName + ("%.2i" % info.groupOrderID)), info.componentID, info.uid, info.pid, info.globalOrderID, 
					 info.cpu, info.mem, info.usedmem / 1024.0 / 1024.0, info.extradata, info.extradata1, 0))
					
					numEntities += info.extradata
					numCells += info.extradata1
				elif info.componentType == LOGINAPP_TYPE:
					print("|-%12s\t%i\t%i\t%i\t%i\t%.2f\t%.2f\t%.2fm\t%i\t\t%i\t\t%i" % \
					((info.componentName + ("%.2i" % info.groupOrderID)), info.componentID, info.uid, info.pid, info.globalOrderID, info.cpu, 
					 info.mem, info.usedmem / 1024.0 / 1024.0, 0, 0, 0))
				else:
					print("|-%12s\t%i\t%i\t%i\t%i\t%.2f\t%.2f\t%.2fm\t%i\t\t%i\t\t%i" % \
					(info.componentName, info.componentID, info.uid, info.pid, info.globalOrderID, info.cpu, 
					 info.mem, info.usedmem / 1024.0 / 1024.0, 0, 0, 0))
					
			
		print('-----------------------------------------------------')
		print("machines: %i, components=%i, numBases=%i, numProxices=%i, numClients=%i, numEntities=%i, numCells=%i." % \
			(len(self.interfaces_groups), numComponent, numBases, numProxices, numClients, numEntities, numCells))
		
class ClusterStartHandler(ClusterControllerHandler):
	def __init__(self, uid, startTemplate, machineIP, cid, gus, kbe_root, kbe_res_path, kbe_bin_path):
		ClusterControllerHandler.__init__(self, uid)
		
		self.startTemplate = startTemplate.split("|")
		self.machineIP = machineIP
		self.cid = cid
		self.gus = gus
		self.kbe_root = kbe_root
		self.kbe_res_path = kbe_res_path
		self.kbe_bin_path = kbe_bin_path
		
	def do(self):
		self.queryAllInterfaces(MACHINES_ADDRESS, MACHINES_QUERY_ATTEMPT_COUNT, MACHINES_QUERY_WAIT_TIME)
		
		print("[curr-online-components:]")
		for ctype in self.interfaces:
			infos = self.interfaces.get(ctype, [])
			print("\t\t%s : %i" % (COMPONENT_NAME[ctype], len(infos)))
			
		interfacesCount = {}
		for ctype in self.startTemplate:
			if ctype not in COMPONENT_NAME2TYPE:
				continue
				
			if ctype in interfacesCount:
				interfacesCount[ctype] += 1
			else:
				infos = self.interfaces.get(COMPONENT_NAME2TYPE[ctype], [])
				interfacesCount[ctype] = 1 + len(infos)
			
		for ctype in self.startTemplate:
			if ctype not in COMPONENT_NAME2TYPE:
				print("not found %s, start failed!" % ctype)
				continue
				
			self.startServer( COMPONENT_NAME2TYPE[ctype], self.cid, self.gus, self.machineIP, self.kbe_root, self.kbe_res_path, self.kbe_bin_path )
		
		
		qcount = 1
		while(True):
			print("query status: %i" % qcount)
			qcount += 1
			
			self.queryAllInterfaces(MACHINES_ADDRESS, MACHINES_QUERY_ATTEMPT_COUNT, MACHINES_QUERY_WAIT_TIME)
			
			waitcount = 0
			for ctype in interfacesCount:
				infos = self.interfaces.get(COMPONENT_NAME2TYPE[ctype], [])
				print("\t\t%s : %i" % (ctype, len(infos)))
				waitcount += interfacesCount[ctype] - len(infos)
			
			if waitcount > 0:
				time.sleep(3)
			else:
				break
		
		print("[all-online-components:]")
		for ctype in self.interfaces:
			infos = self.interfaces.get(ctype, [])
			print("\t\t%s : %i" % (COMPONENT_NAME[ctype], len(infos)))
			
		print("ClusterStartHandler::do: completed!")
			
class ClusterStopHandler(ClusterControllerHandler):
	def __init__(self, uid, startTemplate):
		ClusterControllerHandler.__init__(self, uid)
		
		if len(startTemplate) > 0:
			self.startTemplate = startTemplate.split("|")
		else:
			self.startTemplate = []
	
	def sendStop(self, showDebug):
		interfaces = self.interfaces
		
		if len(self.startTemplate) <= 0:
			for ctype in self.interfaces:
				infos = self.interfaces.get(ctype, [])
				for x in range(0, len(infos)):
					self.startTemplate.append(COMPONENT_NAME[ctype])

		self.interfacesCount = {}
		self.interfacesCount1 = {}
		
		print("online-components:")
		printed = []
		for ctype in self.startTemplate:
			if ctype not in COMPONENT_NAME2TYPE or ctype in NO_SHUTDOWN_COMPONENTS:
				if(ctype not in NO_SHUTDOWN_COMPONENTS):
					print("not found %s, stop failed!" % ctype)
					
				continue
			
			infos = interfaces.get(COMPONENT_NAME2TYPE[ctype], [])
			
			clist = []
			for info in infos: 
				if info.uid == self.uid:
					clist.append(info.componentID)
						
			self.interfacesCount[ctype] = len(clist)

			if ctype in self.interfacesCount1:
				self.interfacesCount1[ctype] += 1
			else:
				self.interfacesCount1[ctype] = 1
			
			if self.interfacesCount1[ctype] > self.interfacesCount[ctype]:
				continue
			
			if ctype not in printed:
				printed.append(ctype)
				if showDebug:
					print("\t\t%s : %i\t%s" % (ctype, len(clist), clist))
			
			# 最好是尽量多的尝试次数，否则可能包未及时恢复造成后续查询错乱
			self.stopServer( COMPONENT_NAME2TYPE[ctype], 0, MACHINES_ADDRESS, 3 )
			
			#print ("ClusterStopHandler::do: stop uid=%s, type=%s, send=%s" % (self.uid, ctype, \
			#	len(self.recvDatas) > 0 and self.recvDatas[0] == b'\x01'))
			
	def do(self):
		qcount = 0
		
		while(True):
			if qcount > 0:
				print("\nquery status: %i" % qcount)
				
			qcount += 1
			
			self.queryAllInterfaces(MACHINES_ADDRESS, MACHINES_QUERY_ATTEMPT_COUNT, MACHINES_QUERY_WAIT_TIME)
			self.sendStop(qcount == 1)
			
			if qcount == 1:
				continue
				
			waitcount = 0
			for ctype in self.interfacesCount:
				if ctype not in COMPONENT_NAME2TYPE or ctype not in self.startTemplate or ctype == "machine":
					continue
			
				infos = self.interfaces.get(COMPONENT_NAME2TYPE[ctype], [])
				
				clist = []
				for info in infos: 
					if info.uid == self.uid:
						clist.append(info.componentID)
						
				print("\t\t%s : %i\t%s" % (ctype, len(clist), clist))
				waitcount += len(clist)

			if waitcount > 0:
				time.sleep(3)
			else:
				break
		
		print("[other-online-components:]")
		for ctype in self.interfaces:
			infos = self.interfaces.get(ctype, [])
			clist = []
			for info in infos: 
				if info.uid == self.uid:
					clist.append(info.componentID)
			print("\t\t%s : %i\t%s" % (COMPONENT_NAME[ctype], len(clist), clist))
			
		print("ClusterStopHandler::do: completed!")

class ClusterLogWatchHandler(ClusterControllerHandler):
	"""
	日志实时输出
	"""
	def __init__(self, uid):
		ClusterControllerHandler.__init__(self, uid)
		
		self.watcher = LoggerWatcher()

	def do(self, receiveLogCB = None):
		self.queryAllInterfaces(MACHINES_ADDRESS, MACHINES_QUERY_ATTEMPT_COUNT, MACHINES_QUERY_WAIT_TIME)
		
		infos = self.getComponentInfos( LOGGER_TYPE )
		if not infos:
			print( "Error: no logger found!" )
			return
		
		logger = None
		for info in infos:
			if info.uid == self.uid:
				logger = info
				break
		
		if not logger:
			print( "Error: no logger found! (uid = %s)" % self.uid )
			return
		
		print( "Logger found: internal %s:%s, external %s:%s" % ( logger.intaddr, logger.intport, logger.extaddr, logger.extport ) )
		
		self.watcher.connect( logger.extaddr, logger.extport )
		self.watcher.registerToLogger( self.uid )
		
		def onReceivedLog( logs ):
			for e in logs:
				print( e )
		
		if receiveLogCB is None:
			self.watcher.receiveLog(onReceivedLog, True)
		else:
			self.watcher.receiveLog(receiveLogCB, True)

class ClusterSendLogHandler(ClusterControllerHandler):
	"""
	日志实时输出
	"""
	def __init__(self, uid, type, logStr):
		ClusterControllerHandler.__init__(self, uid)
		
		self.watcher = LoggerWatcher()
		self.uid = uid
		self.type = type
		self.logStr = logStr

	def do(self):
		self.queryAllInterfaces(MACHINES_ADDRESS, MACHINES_QUERY_ATTEMPT_COUNT, MACHINES_QUERY_WAIT_TIME)
		
		infos = self.getComponentInfos( LOGGER_TYPE )
		if not infos:
			print( "Error: no logger found!" )
			return
		
		logger = None
		for info in infos:
			if info.uid == self.uid:
				logger = info
				break
		
		if not logger:
			print( "Error: no logger found! (uid = %s)" % self.uid )
			return
		
		print( "Logger found: internal %s:%s, external %s:%s" % ( logger.intaddr, logger.intport, logger.extaddr, logger.extport ) )
		
		self.watcher.connect( logger.extaddr, logger.extport )
		self.watcher.sendLog( self.uid, self.type, self.logStr )
		time.sleep( 1.0 )
		self.watcher.close()
		

class ClusterSaveProcessHandler(ClusterControllerHandler):
	"""
	把当前的服务器运行状态存储到配置中，以便在下次可以使用此进程直接启动
	"""
	# 有效的componentType
	VALIDATE_CT = set( [
			DBMGR_TYPE,
			LOGINAPP_TYPE,
			BASEAPPMGR_TYPE,
			CELLAPPMGR_TYPE,
			CELLAPP_TYPE,
			BASEAPP_TYPE,
			INTERFACES_TYPE,
			LOGGER_TYPE,
		] )

	def __init__(self, uid, filename):
		"""
		"""
		ClusterControllerHandler.__init__(self, uid)
		self.filename = filename
	
	def do(self):
		"""
		"""
		self.queryAllInterfaces(MACHINES_ADDRESS, MACHINES_QUERY_ATTEMPT_COUNT, MACHINES_QUERY_WAIT_TIME)
		
		cper = configparser.ConfigParser()
		for i in range( COMPONENT_END_TYPE ):
			if i in self.VALIDATE_CT:
				cper.add_section( COMPONENT_NAME[i] )
		
		# 计数器
		t2c = [0,] * len(COMPONENT_NAME)
		
		# ip, cid, gus
		vt = "%s, %s, %s"
		for machineID, infos in self.interfaces_groups.items():
			for info in infos:
				if info.componentType not in self.VALIDATE_CT:
					continue
					
				t2c[info.componentType] += 1
				v = vt % (info.intaddr, info.componentID, info.genuuid_sections)
				cper.set(COMPONENT_NAME[info.componentType], "item_%s" % t2c[info.componentType], v)
		
		if self.filename:
			fs = open( self.filename, "wb" )
			cper.write( fs )
			fs.close()
		else:
			cper.write( sys.stdout )


class ClusterLoadProcessHandler(ClusterControllerHandler):
	"""
	从配置文件中启动服务器
	"""
	# 有效的componentType
	VALIDATE_CT = set( [
			DBMGR_TYPE,
			LOGINAPP_TYPE,
			BASEAPPMGR_TYPE,
			CELLAPPMGR_TYPE,
			CELLAPP_TYPE,
			BASEAPP_TYPE,
			INTERFACES_TYPE,
			LOGGER_TYPE,
		] )

	def __init__(self, uid, filename):
		"""
		"""
		ClusterControllerHandler.__init__(self, uid)
		self.filename = filename
	
	def do(self):
		"""
		"""
		cper = configparser.ConfigParser()
		cper.read(filename)
		
		expectCount = [0] * COMPONENT_END_TYPE
		
		SIGNLE_CT = [
				DBMGR_TYPE,
				BASEAPPMGR_TYPE,
				CELLAPPMGR_TYPE,
				INTERFACES_TYPE,
				LOGGER_TYPE,
			]

		for ct in SIGNLE_CT:
			secName = COMPONENT_NAME[ct]
			optName = "item_1"
			if cper.has_option( secName, optName ):
				expectCount[ct] += 1
				targetIP, cid, gus = cper.get(secName, optName).split(",")
				targetIP, cid, gus = targetIP.strip(), int(cid), int(gus)
				if gus <= 0:
					gus = self.makeGUS(ct)

				print( "run '%s' in '%s', uid = %s, cid = %s, gus = %s" % (secName, targetIP, self.uid, cid, gus) )
				self.startServer( ct, cid, gus, targetIP,"","","" )

		MUTIL_CT = [
				LOGINAPP_TYPE,
				CELLAPP_TYPE,
				BASEAPP_TYPE,
			]

		for ct in MUTIL_CT:
			secName = COMPONENT_NAME[ct]
			idx = 1
			while True:
				optName = "item_%s" % idx
				idx += 1
				if cper.has_option( secName, optName ):
					expectCount[ct] += 1
					targetIP, cid, gus = cper.get(secName, optName).split(",")
					targetIP, cid, gus = targetIP.strip(), int(cid), int(gus)
					if gus <= 0:
						gus = self.makeGUS(ct)

					print( "run '%s' in '%s', uid = %s, cid = %s, gus = %s" % (secName, targetIP, self.uid, cid, gus) )
					self.startServer( ct, cid, gus, targetIP,"","","" )
				else:
					break
		
		print("")
		
		queryCount = 0
		while True:
			queryCount += 1
			print( "query status: %s" % queryCount )
			self.queryAllInterfaces(MACHINES_ADDRESS, MACHINES_QUERY_ATTEMPT_COUNT, MACHINES_QUERY_WAIT_TIME)
			currentCount = [0] * COMPONENT_END_TYPE
			for machineID, infos in self.interfaces_groups.items():
				for info in infos:
					if info.componentType not in self.VALIDATE_CT:
						continue
					currentCount[info.componentType] += 1
			
			for i in range( len( expectCount ) ):
				if expectCount[i] <= 0:
					continue
				print( "\t%11s: %s/%s" % ( COMPONENT_NAME[i], currentCount[i], expectCount[i] ) )
			
			print("")
			
			if currentCount == expectCount:
				break

class ClusterStartServerHandler(ClusterControllerHandler):
	"""
	在本地快速开始一组服务器（用于测试）
	"""
	# 要运行的componentType
	VALIDATE_CT = [
			LOGGER_TYPE,
			INTERFACES_TYPE,
			DBMGR_TYPE,
			BASEAPPMGR_TYPE,
			CELLAPPMGR_TYPE,
			CELLAPP_TYPE,
			BASEAPP_TYPE,
			LOGINAPP_TYPE,
		]

	def __init__(self, uid, machineIP):
		"""
		"""
		ClusterControllerHandler.__init__(self, uid)
		self.machineIP = machineIP
	
	def do(self):
		"""
		"""
		for ct in self.VALIDATE_CT:
			secName = COMPONENT_NAME[ct]
			cid = self.makeCID( ct )
			gus = self.makeGUS( ct )
			print( "run '%s' in '%s', uid = %s, cid = %s, gus = %s" % (secName, self.machineIP, self.uid, cid, gus) )
			self.startServer( ct, cid, gus, self.machineIP,"","","" )

		queryCount = 0
		expectCount = [0] * COMPONENT_END_TYPE
		for ct in self.VALIDATE_CT:
			expectCount[ct] = 1
		
		while True:
			queryCount += 1
			print( "query status: %s" % queryCount )
			self.queryAllInterfaces(MACHINES_ADDRESS, MACHINES_QUERY_ATTEMPT_COUNT, MACHINES_QUERY_WAIT_TIME)
			currentCount = [0] * COMPONENT_END_TYPE
			for machineID, infos in self.interfaces_groups.items():
				for info in infos:
					if info.componentType not in self.VALIDATE_CT:
						continue
					currentCount[info.componentType] += 1
			
			for i in range( len( expectCount ) ):
				if expectCount[i] <= 0:
					continue
				print( "\t%11s: %s/%s" % ( COMPONENT_NAME[i], currentCount[i], expectCount[i] ) )
			
			print("")
			
			if currentCount == expectCount:
				break



if __name__ == "__main__":

	clusterHandler = None

	if len(sys.argv)  >= 2:
		cmdType = sys.argv[1]
		
		if cmdType == "start":
			uid = -1
			
			machineip = "127.0.0.1"
			if len(sys.argv) >= 3:
				machineip = sys.argv[2]

			if len(sys.argv) > 3:
				if sys.argv[3].isdigit():
					uid = int( sys.argv[3] )
				else:
					print("syntax: cluster_controller.py start [machine-ip] [user-id]")
					print("exp: cluster_controller.py start 10.11.12.13 501")
					exit(1)
			else:
				uid = getDefaultUID()
			
			if uid < 0:
				print("syntax: cluster_controller.py start [machine-ip] [user-id]")
				print("exp: cluster_controller.py start 10.11.12.13 501")
				exit(1)
			
			clusterHandler = ClusterStartServerHandler(uid, machineip)

		elif cmdType == "startprocess":
			templatestr = "dbmgr|baseappmgr|cellappmgr|baseapp|cellapp|loginapp|interfaces|logger"
			uid = -1
			
			# cluster_controller.py startprocess dbmgr 123456789012345678 123 [10.11.12.13] [uid]
			if len(sys.argv) < 6:
				print("syntax: cluster_controller.py startprocess componentName cid gus machine-ip [user-id]")
				print("exp: cluster_controller.py startprocess dbmgr 123456789012345678 123 10.11.12.13")
				exit(1)
			
			if len(sys.argv) > 6:
				_, _, componentName, cid, gus, machineip, uid = sys.argv
				uid = int(uid)
			else:
				_, _, componentName, cid, gus, machineip = sys.argv
				uid = getDefaultUID()
			
			if componentName not in templatestr:
				print("Error: component name invalid. refer to:", templatestr)
				exit(1)
			
			cid = int(cid)
			gus = int(gus)
			
			clusterHandler = ClusterStartHandler(uid, componentName, machineip, cid, gus, "","","")

		elif cmdType == "stop":
			templatestr = ""
			uid = -1
			
			if len(sys.argv) >= 3:
				if sys.argv[2].isdigit():
					uid = sys.argv[2]
				else:
					templatestr = sys.argv[2]

			if len(sys.argv) == 4:
				if sys.argv[3].isdigit():
					uid = sys.argv[3]
				else:
					templatestr = sys.argv[3]
					
			uid = int(uid)
			if uid < 0:
				uid = getDefaultUID()
				
			clusterHandler = ClusterStopHandler(uid, templatestr)
		elif cmdType == "shutdown":
			templatestr = ""
			uid = -1
			
			if len(sys.argv) >= 3:
				if sys.argv[2].isdigit():
					uid = sys.argv[2]

			uid = int(uid)
			if uid < 0:
				uid = getDefaultUID()
			
			NO_SHUTDOWN_COMPONENTS = []
			clusterHandler = ClusterStopHandler(uid, templatestr)
		elif cmdType == "console":
			consoleType = ""
			uid = -1
			
			if len(sys.argv) >= 3:
				if sys.argv[2].isdigit():
					uid = sys.argv[2]
				else:
					consoleType = sys.argv[2]

			if len(sys.argv) == 4:
				if sys.argv[3].isdigit():
					uid = sys.argv[3]
				else:
					consoleType = sys.argv[3]
					
			uid = int(uid)
			if uid < 0:
				uid = getDefaultUID()
				
			clusterHandler = ClusterConsoleHandler(uid, consoleType)
		elif cmdType == "query":
			uid = -1

			if len(sys.argv) >= 3:
				if sys.argv[2].isdigit():
					uid = sys.argv[2]

			uid = int(uid)
			if uid < 0:
				uid = getDefaultUID()
					
			clusterHandler = ClusterQueryHandler(uid)
		
		# python cluster_controller.py showlog uid
		elif cmdType == "showlog":
			uid = -1

			if len(sys.argv) >= 3:
				if sys.argv[2].isdigit():
					uid = sys.argv[2]
				else:
					print("syntax: cluster_controller.py showlog [uid]")
					print("exp: cluster_controller.py showlog 501")
					exit(1)

			uid = int(uid)
			if uid < 0:
				uid = getDefaultUID()

			clusterHandler = ClusterLogWatchHandler(uid)
		
		# python cluster_controller.py sendlog uid NORMAL|INFO|DEBUG|ERROR|WARNING logstr
		elif cmdType == "sendlog":
			if len(sys.argv) < 4:
				print("syntax: cluster_controller.py sendlog [uid] NORMAL|INFO|DEBUG|ERROR|WARNING logstr")
				print("exp: cluster_controller.py sendlog 501 DEBUG \"send log test\"")
				exit(1)
			
			uid = -1
			
			logType = ""
			logStr = ""
			if sys.argv[2].isdigit():
				uid = sys.argv[2]
				uid = int(uid)
				if len(sys.argv) < 5:
					print("syntax: cluster_controller.py sendlog [uid] NORMAL|INFO|DEBUG|ERROR|WARNING logstr")
					print("exp: cluster_controller.py sendlog 501 DEBUG \"send log test\"")
					exit(1)
				else:
					logType = sys.argv[3]
					logStr = sys.argv[4]
			else:
				logType = sys.argv[2]
				logStr = sys.argv[3]

			if uid <= 0:
				uid = getDefaultUID()

			clusterHandler = ClusterSendLogHandler(uid, logType, logStr)
		elif cmdType == "save":
			uid = -1

			filename = None
			if len(sys.argv) < 3:
				uid = getDefaultUID()
			elif len(sys.argv) == 3:
				if sys.argv[2].isdigit():
					uid = sys.argv[2]
				else:
					filename = sys.argv[2]
			else:
				if sys.argv[2].isdigit():
					uid = sys.argv[2]
				else:
					print("syntax: cluster_controller.py save [uid] [save_file.cfg]")
					print("exp: cluster_controller.py save 501 kbengine_process.cfg")
					exit(1)
				filename = sys.argv[3]

			uid = int(uid)
			if uid < 0:
				print("syntax: cluster_controller.py save [uid] [save_file.cfg]")
				print("exp: cluster_controller.py save 501 kbengine_process.cfg")
				exit(1)

			clusterHandler = ClusterSaveProcessHandler(uid, filename)
		elif cmdType == "load":
			uid = -1
			
			if len(sys.argv) < 3:
				print("syntax: cluster_controller.py load [uid] save_file.cfg")
				print("exp: cluster_controller.py load 501 kbengine_process.cfg")
				exit(1)
			
			if len(sys.argv) == 3:
				if sys.argv[2].isdigit():
					uid = sys.argv[2]
				else:
					uid = getDefaultUID()
					filename = sys.argv[2]
			else:
				if sys.argv[2].isdigit():
					uid = sys.argv[2]
				else:
					print("syntax: cluster_controller.py load [uid] save_file.cfg")
					print("exp: cluster_controller.py load 501 kbengine_process.cfg")
					exit(1)
				filename = sys.argv[3]
				
			uid = int(uid)
			if uid < 0:
				print("syntax: cluster_controller.py load [uid] save_file.cfg")
				print("exp: cluster_controller.py load 501 kbengine_process.cfg")
				exit(1)

			clusterHandler = ClusterLoadProcessHandler(uid, filename)
		else:
			uid = -1

			if len(sys.argv) >= 2:
				if sys.argv[1].isdigit():
					uid = sys.argv[1]

			uid = int(uid)

			#如果没有给UID参数，则默认为0，查询所有机器
			if uid < 0:
				uid = 0

			clusterHandler = ClusterQueryHandler(uid)
	else:
		clusterHandler = ClusterQueryHandler(0)
			
	if clusterHandler is not None: 
		clusterHandler.do()
