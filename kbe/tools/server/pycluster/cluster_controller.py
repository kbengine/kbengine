# -*- coding: utf-8 -*-

import socket, time
import sys
import struct
import traceback
import select
import getpass 
import time
import os

if sys.hexversion >= 0x03000000:
	import io
else:
	import StringIO

host = '' # Bind to all interfaces

MachineInterface_onFindInterfaceAddr = 1
MachineInterface_startserver = 2
MachineInterface_stopserver = 3
MachineInterface_onQueryAllInterfaceInfos = 4
MachineInterface_onQueryMachines = 5

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


# ComponentName to type
COMPONENT_NAME2TYPE = {
	"unknown"		: UNKNOWN_COMPONENT_TYPE,
	"dbmgr"			: DBMGR_TYPE,
	"loginapp"		: LOGINAPP_TYPE,
	"baseappmgr"	: BASEAPPMGR_TYPE,
	"cellappmgr"	: CELLAPPMGR_TYPE,
	"cellapp" 		: CELLAPP_TYPE,
	"baseapp" 		: BASEAPP_TYPE,
	"client" 		: CLIENT_TYPE,
	"machine"		: MACHINE_TYPE,
	"console" 		: CONSOLE_TYPE,
	"logger" 		: LOGGER_TYPE,
	"bots" 			: BOTS_TYPE,
	"watcher" 		: WATCHER_TYPE,
	"interfaces" 	: INTERFACES_TYPE,
}

# Component name
COMPONENT_NAME = (
	"unknown",
	"dbmgr",
	"loginapp",
	"baseappmgr",
	"cellappmgr",
	"cellapp",
	"baseapp",
	"client",
	"machine",
	"console",
	"logger",
	"bots",
	"watcher",
	"interfaces",
)

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



class ComponentInfo( object ):
	"""
	"""
	def __init__( self, streamStr = None ):
		"""
		"""
		if streamStr:
			self.initFromStream( streamStr )
	
	def initFromStream( self, streamStr ):
		"""
		"""
		i = 4
		self.uid = struct.unpack("i", streamStr[0:i])[0]
		
		ii = i
		for x in streamStr[i:]:
			if type(x) == str:
				if ord(x) == 0:
					break
			else:
				if x == 0:
					break

			ii += 1

		self.username = streamStr[i: ii];
		if type(self.username) == 'bytes':
			self.username = self.username.decode()
                            
		ii += 1

		self.componentType = struct.unpack("i", streamStr[ii : ii + 4])[0]
		self.componentName = COMPONENT_NAME[self.componentType]
		ii += 4
		
		self.componentID = struct.unpack("Q", streamStr[ii : ii + 8])[0]
		ii += 16

		self.globalOrderID = struct.unpack("i", streamStr[ii : ii + 4])[0]
		ii += 4

		self.groupOrderID = struct.unpack("i", streamStr[ii : ii + 4])[0]
		ii += 4

		#self.intaddr = struct.unpack("I", streamStr[ii : ii + 4])[0]
		self.intaddr = socket.inet_ntoa(streamStr[ii : ii + 4])
		ii += 4

		self.intport = struct.unpack(">H", streamStr[ii : ii + 2])[0]
		ii += 2

		#self.extaddr = struct.unpack("I", streamStr[ii : ii + 4])[0]
		self.extaddr = socket.inet_ntoa(streamStr[ii : ii + 4])
		ii += 4

		self.extport = struct.unpack(">H", streamStr[ii : ii + 2])[0]
		ii += 2
		
		# get extaddrEx
		i1 = ii
		for x in streamStr[ii:]:
			if type(x) == str:
				if ord(x) == 0:
					break
			else:
				if x == 0:
					break

			ii += 1

		self.extaddrEx = streamStr[i1: ii];
		if type(self.extaddrEx) == 'bytes':
			self.extaddrEx = extaddrEx.decode()

		ii += 1

		self.pid = struct.unpack("I", streamStr[ii : ii + 4])[0]
		ii += 4
		
		self.cpu = struct.unpack("f", streamStr[ii : ii + 4])[0]
		ii += 4

		self.mem = struct.unpack("f", streamStr[ii : ii + 4])[0]
		ii += 4

		self.usedmem = struct.unpack("I", streamStr[ii : ii + 4])[0]
		ii += 4

		self.state = struct.unpack("b", streamStr[ii : ii + 1])[0]
		ii += 1
		
		self.machineID = struct.unpack("I", streamStr[ii : ii + 4])[0]
		ii += 4
		
		self.extradata = struct.unpack("Q", streamStr[ii : ii + 8])[0]
		ii += 8

		self.extradata1 = struct.unpack("Q", streamStr[ii : ii + 8])[0]
		ii += 8
		
		self.extradata2 = struct.unpack("Q", streamStr[ii : ii + 8])[0]
		ii += 8

		self.extradata3 = struct.unpack("Q", streamStr[ii : ii + 8])[0]
		ii += 8
		
		self.backaddr = struct.unpack("I", streamStr[ii : ii + 4])[0]
		ii += 4

		self.backport = struct.unpack("H", streamStr[ii : ii + 2])[0]
		ii += 2
		
		#print("%s, uid=%i, cID=%i, gid=%i, groupid=%i, uname=%s" % (COMPONENT_NAME[self.componentType], \
		#	self.uid, self.componentID, self.globalOrderID, self.groupOrderID, self.username))
		

class ClusterControllerHandler:
	def __init__(self):
		self.udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
		self.udp_socket.bind((host, 0))
		
		self._tcp_socket = socket.socket()
		self.resetPacket()
		
		self._interfaces = {}
		self._interfaces_groups = {}
		self._interfaces_groups_uid = {}
		self._machines = []
		
	def do(self):
		pass
	
	def resetPacket(self):
		self.recvDatas = []

		if sys.hexversion >= 0x03000000:
			self.postDatas = eval("b''")
		else:
			self.postDatas = ""

	def sendto(self, ip = "<broadcast>", trycount = 1, timeout = 1):
		self.writePacket("H", socket.htons(self.udp_socket.getsockname()[1]))
		
		_udp_broadcast_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
		_udp_broadcast_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
		
		if ip == "<broadcast>":
			_udp_broadcast_socket.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
			_udp_broadcast_socket.sendto(self.postDatas, ('255.255.255.255', 20086))
		else:
			_udp_broadcast_socket.sendto(self.postDatas, (ip, 20086))
		
		self.resetPacket()
		
		self.udp_socket.settimeout(timeout)
		dectrycount = trycount
		
		while(dectrycount > 0):
			try:
				dectrycount = trycount
				recvdata, address = self.udp_socket.recvfrom(10240)
				self.recvDatas.append(recvdata)
				#print ("received %r from %r" % (self.recvDatas, address))
			except socket.timeout: 
				dectrycount -= 1
				
				if dectrycount <= 0:
					#print ("recvfrom timeout!")
					break
			except (KeyboardInterrupt, SystemExit):
				raise
			except:
				traceback.print_exc()
				break

	def writePacket(self, fmt, data):
		self.postDatas += struct.pack(fmt, data)
     
	def queryAllInterfaces(self):
		# print("queryAllInterfaces...")
		self.resetPacket()
		self.writePacket("H", MachineInterface_onQueryAllInterfaceInfos)
		self.writePacket("H", 6 + len(getpass.getuser().encode()) + 1)
		self.writePacket("i", self.uid)

		for x in getpass.getuser().encode():
			if type(x) == str:
				self.writePacket("B", ord(x))
			else:
				self.writePacket("B", x)
                           
		self.writePacket("B", 0)
		self.sendto()
		self.parseQueryDatas()

	def queryMachines(self):
		#print("queryMachines...")
		self.resetPacket()
		self.writePacket("H", MachineInterface_onQueryMachines)
		self.writePacket("H", 6 + len(getpass.getuser().encode()) + 1)
		self.writePacket("i", self.uid)

		for x in getpass.getuser().encode():
			if type(x) == str:
				self.writePacket("B", ord(x))
			else:
				self.writePacket("B", x)
                           
		self.writePacket("B", 0)
		self.sendto()
		self.parseQueryDatas()

	def parseQueryDatas(self):
		self._interfaces = {}
		if len(self.recvDatas) == 0:
			return
		
		count = 0

		while(count < len(self.recvDatas)):
			cinfo = ComponentInfo( self.recvDatas[count] )
			count += 1

			componentInfos = self._interfaces.get(cinfo.componentType)
			if componentInfos is None:
				componentInfos = []
				self._interfaces[cinfo.componentType] = componentInfos
			
			found = False
			for info in componentInfos:
				if info.componentID == cinfo.componentID and info.pid == cinfo.pid:
					found = True
					break
			
			if not found:
				componentInfos.append(cinfo)
		
		self._interfaces_groups = {}
		self._interfaces_groups_uid = {}
		for ctype in self._interfaces:
			infos = self._interfaces.get(ctype, [])
			for info in infos:
				machineID = info.machineID
				
				gourps = self._interfaces_groups.get(machineID, [])
				if machineID not in self._interfaces_groups:
					self._interfaces_groups[machineID] = gourps
					self._interfaces_groups_uid[machineID] = []
					
				# 如果pid与machineID相等，说明这个是machine进程
				if info.pid != machineID:
					gourps.append(info)
					if info.uid not in self._interfaces_groups_uid[machineID]:
						self._interfaces_groups_uid[machineID].append(info.uid)
				else:
					# 是machine进程，把它放在最前面，并且加到machines列表中
					gourps.insert(0, info)
					self._machines.append( infos )

	def getMachine( self, ip ):
		"""
		通过ip地址找到对应的machine的info
		"""
		for info in self.self._machines:
			if info.intaddr == ip:
				return info
		return None
	
	def getComponentInfos( self, componentType ):
		"""
		获取某一类型的组件信息
		"""
		return self._interfaces.get( componentType, [] )

class ClusterConsoleHandler(ClusterControllerHandler):
	def __init__(self, uid, consoleType):
		ClusterControllerHandler.__init__(self)
		self.uid = uid
		self.consoleType = consoleType
		
	def do(self):
		self._interfaces_groups = {}
		print("finding(" + self.consoleType  + ")...")
		self.queryAllInterfaces()
		
		for machineID in self._interfaces_groups:
			infos = self._interfaces_groups.get(machineID, [])
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
		ClusterControllerHandler.__init__(self)
		self.uid = uid
		
	def do(self):
		self._interfaces_groups = {}
		self.queryAllInterfaces()
		
		numBases = 0
		numEntities = 0
		numClients = 0
		numProxices = 0
		numCells = 0
		numComponent = 0
		
		for machineID in self._interfaces_groups:
			infos = self._interfaces_groups.get(machineID, [])
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
					print("|-%12s%i\t%i\t%i\t%i\t%i\t%.2f\t%.2f\t%.2fm\tbases=%i\t\tclients=%i\tproxices=%i" % \
					(info.componentName, info.groupOrderID, info.componentID, info.uid, info.pid, info.globalOrderID, 
					 info.cpu, info.mem, info.usedmem / 1024.0 / 1024.0, info.extradata, info.extradata1, info.extradata2))

					numBases += info.extradata
					numClients += info.extradata1
					numProxices += info.extradata2
		
				elif info.componentType == CELLAPP_TYPE:
					print("|-%12s%i\t%i\t%i\t%i\t%i\t%.2f\t%.2f\t%.2fm\tentities=%i\tcells=%i\t\t%i" % \
					(info.componentName, info.groupOrderID, info.componentID, info.uid, info.pid, info.globalOrderID, 
					 info.cpu, info.mem, info.usedmem / 1024.0 / 1024.0, info.extradata, info.extradata1, 0))
					
					numEntities += info.extradata
				else:
					print("|-%12s\t%i\t%i\t%i\t%i\t%.2f\t%.2f\t%.2fm\t%i\t\t%i\t\t%i" % \
					(info.componentName, info.componentID, info.uid, info.pid, info.globalOrderID, info.cpu, 
					 info.mem, info.usedmem / 1024.0 / 1024.0, 0, 0, 0))
					
			
		print('-----------------------------------------------------')
		print("machines: %i, components=%i, numBases=%i, numProxices=%i, numClients=%i, numEntities=%i, numCells=%i." % \
			(len(self._interfaces_groups), numComponent, numBases, numProxices, numClients, numEntities, numCells))
		
class ClusterStartHandler(ClusterControllerHandler):
	def __init__(self, uid, startTemplate, machineIP, cid, gus):
		ClusterControllerHandler.__init__(self)
		
		self.uid = uid
		self.startTemplate = startTemplate.split("|")
		self.machineIP = machineIP
		self.cid = cid
		self.gus = gus
		
	def do(self):
		self.queryAllInterfaces()
		
		print("[curr-online-components:]")
		for ctype in self._interfaces:
			infos = self._interfaces.get(ctype, [])
			print("\t\t%s : %i" % (COMPONENT_NAME[ctype], len(infos)))
			
		interfacesCount = {}
		for ctype in self.startTemplate:
			if ctype not in COMPONENT_NAME2TYPE:
				continue
				
			if ctype in interfacesCount:
				interfacesCount[ctype] += 1
			else:
				infos = self._interfaces.get(COMPONENT_NAME2TYPE[ctype], [])
				interfacesCount[ctype] = 1 + len(infos)
			
		for ctype in self.startTemplate:
			if ctype not in COMPONENT_NAME2TYPE:
				print("not found %s, start failed!" % ctype)
				continue
				
			self.writePacket("H", MachineInterface_startserver)
			self.writePacket("H", 20)  # package size
			self.writePacket("i", self.uid)
			self.writePacket("i", COMPONENT_NAME2TYPE[ctype])
			self.writePacket("Q", self.cid)
			self.writePacket("h", self.gus)
			self.sendto( self.machineIP )
		
		
		qcount = 1
		while(True):
			print("query status: %i" % qcount)
			qcount += 1
			
			self.queryAllInterfaces()
			
			waitcount = 0
			for ctype in interfacesCount:
				infos = self._interfaces.get(COMPONENT_NAME2TYPE[ctype], [])
				print("\t\t%s : %i" % (ctype, len(infos)))
				waitcount += interfacesCount[ctype] - len(infos)
			
			if waitcount > 0:
				time.sleep(3)
			else:
				break
		
		print("[all-online-components:]")
		for ctype in self._interfaces:
			infos = self._interfaces.get(ctype, [])
			print("\t\t%s : %i" % (COMPONENT_NAME[ctype], len(infos)))
			
		print("ClusterStartHandler::do: completed!")
			
class ClusterStopHandler(ClusterControllerHandler):
	def __init__(self, uid, startTemplate):
		ClusterControllerHandler.__init__(self)
		self.uid = uid
		
		if len(startTemplate) > 0:
			self.startTemplate = startTemplate.split("|")
		else:
			self.startTemplate = []
	
	def sendStop(self, showDebug):
		interfaces = self._interfaces
		
		if len(self.startTemplate) <= 0:
			for ctype in self._interfaces:
				infos = self._interfaces.get(ctype, [])
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
				
			self.writePacket("H", MachineInterface_stopserver)
			self.writePacket("H", 10)
			self.writePacket("i", self.uid)
			self.writePacket("i", COMPONENT_NAME2TYPE[ctype])
			self.sendto()
			
			#print ("ClusterStopHandler::do: stop uid=%s, type=%s, send=%s" % (self.uid, ctype, \
			#	len(self.recvDatas) > 0 and self.recvDatas[0] == b'\x01'))
			
	def do(self):
		qcount = 0
		
		while(True):
			if qcount > 0:
				print("\nquery status: %i" % qcount)
				
			qcount += 1
			
			self.queryAllInterfaces()
			self.sendStop(qcount == 1)
			
			if qcount == 1:
				continue
				
			waitcount = 0
			for ctype in self.interfacesCount:
				if ctype not in COMPONENT_NAME2TYPE or ctype not in self.startTemplate or ctype == "machine":
					continue
			
				infos = self._interfaces.get(COMPONENT_NAME2TYPE[ctype], [])
				
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
		for ctype in self._interfaces:
			infos = self._interfaces.get(ctype, [])
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
		ClusterControllerHandler.__init__(self)
		
		self.watcher = LoggerWatcher()
		self.uid = uid

	def do(self):
		self.queryAllInterfaces()
		
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
		
		self.watcher.receiveLog(onReceivedLog, True)
		
class ClusterSendLogHandler(ClusterControllerHandler):
	"""
	日志实时输出
	"""
	def __init__(self, uid, type, logStr):
		ClusterControllerHandler.__init__(self)
		
		self.watcher = LoggerWatcher()
		self.uid = uid
		self.type = type
		self.logStr = logStr

	def do(self):
		self.queryAllInterfaces()
		
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
		

def getDefaultUID():
	"""
	"""
	uid = -1
	if uid < 0:
		uid = int(os.environ.get('uid', -1))
	
	try:
		if uid < 0:
			uid = os.getuid()
	except:
		pass

	try:
		if uid < 0:
			import pwd
			pw = pwd.getpwnam(getpass.getuser())
			uid = pw.pw_uid
	except:
		pass
	
	if uid == -1:
		print("\n[ERROR]: UID is not set, The current is -1. Please refer to the http://kbengine.org/docs/installation.html " \
			"environment variable settings, about UID!\n Or using \"cluster_controller.py stop [UID]\" to stop the servers.\n")

	return uid
	




if __name__ == "__main__":

	clusterHandler = None

	if len(sys.argv)  >= 2:
		cmdType = sys.argv[1]
		
		if cmdType == "start":
			templatestr = "dbmgr|baseappmgr|cellappmgr|baseapp|cellapp|loginapp|interfaces|logger|bots"
			uid = -1
			
			# cluster_controller.py start dbmgr 123456789012345678 123 [10.11.12.13] [uid]
			if len(sys.argv) < 6:
				print("syntax: cluster_controller.py start componentName cid gus machine-ip [user-id]")
				print("exp: cluster_controller.py start dbmgr 123456789012345678 123 10.11.12.13")
				exit(1)
			
			if len(sys.argv) > 6:
				_, _, componentName, cid, gus, machineip, uid = sys.argv
				uid = int(uid)
			else:
				_, _, componentName, cid, gus, machineip = sys.argv
				uid = getDefaultUID()
			
			if componentName not in COMPONENT_NAME:
				print("Error: component name invalid. refer to:", templatestr)
				exit(1)
			
			cid = int(cid)
			gus = int(gus)
			
			clusterHandler = ClusterStartHandler(uid, componentName, machineip, cid, gus)

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
			
			clusterHandler = ClusterStartHandler(uid, componentName, machineip, cid, gus)

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
