# -*- coding: utf-8 -*-

import socket
import sys
import os
import struct
import traceback
import select
import getpass 
import time

host = '' # Bind to all interfaces

MachineInterface_onFindInterfaceAddr = 1
MachineInterface_startserver = 2
MachineInterface_stopserver = 3
MachineInterface_onQueryAllInterfaceInfos = 4

# 
UNKNOWN_COMPONENT_TYPE	= 0
DBMGR_TYPE				= 1
LOGINAPP_TYPE			= 2
BASEAPPMGR_TYPE			= 3
CELLAPPMGR_TYPE			= 4
CELLAPP_TYPE			= 5
BASEAPP_TYPE			= 6
CLIENT_TYPE				= 7
MACHINE_TYPE			= 8
CONSOLE_TYPE			= 9
LOGGER_TYPE				= 10
BOTS_TYPE				= 11
WATCHER_TYPE			= 12
INTERFACES_TYPE			= 13
COMPONENT_END_TYPE		= 16

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

class ClusterControllerHandler:
	def __init__(self):
		self.udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
		self.udp_socket.bind((host, 0))
		
		self._tcp_socket = socket.socket()
		self.resetPacket()
		
		self._interfaces = {}
		self._interfaces_groups = {}
		self._interfaces_groups_uid = {}
		
	def do(self):
		pass
	
	def resetPacket(self):
		self.recvDatas = []
		self.postDatas = b""
		
	def sendto(self, trycount = 1, timeout = 3):
		self.writePacket("H", socket.htons(self.udp_socket.getsockname()[1]))
		
		_udp_broadcast_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
		_udp_broadcast_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
		_udp_broadcast_socket.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
		_udp_broadcast_socket.sendto(self.postDatas, ('255.255.255.255', 20086))
		#print("posted udp broadcast(size=%i), waiting for recv..." % len(self.postDatas))
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
		
		self._interfaces = {}
		if len(self.recvDatas) == 0:
			return
		
		count = 0

		while(count < len(self.recvDatas)):
			i = 4
			uid = struct.unpack("i", self.recvDatas[count][0:i])[0]
			
			ii = i
			for x in self.recvDatas[count][i:]:
				if type(x) == str:
					if ord(x) == 0:
						break
				else:
					if x == 0:
						break

				ii += 1

			username = self.recvDatas[count][i: ii];
			if type(username) == 'bytes':
				username = username.decode()
                                
			ii += 1

			componentType = struct.unpack("i", self.recvDatas[count][ii : ii + 4])[0]
			ii += 4
			
			componentID = struct.unpack("Q", self.recvDatas[count][ii : ii + 8])[0]
			ii += 16

			globalorderid = struct.unpack("i", self.recvDatas[count][ii : ii + 4])[0]
			ii += 4

			grouporderid = struct.unpack("i", self.recvDatas[count][ii : ii + 4])[0]
			ii += 4

			intaddr = struct.unpack("I", self.recvDatas[count][ii : ii + 4])[0]
			ii += 4

			intport = struct.unpack("H", self.recvDatas[count][ii : ii + 2])[0]
			ii += 2

			extaddr = struct.unpack("I", self.recvDatas[count][ii : ii + 4])[0]
			ii += 4

			extport = struct.unpack("H", self.recvDatas[count][ii : ii + 2])[0]
			ii += 2
			
			# get extaddrEx
			i1 = ii
			for x in self.recvDatas[count][ii:]:
				if type(x) == str:
					if ord(x) == 0:
						break
				else:
					if x == 0:
						break

				ii += 1

			extaddrEx = self.recvDatas[count][i1: ii];
			if type(extaddrEx) == 'bytes':
				extaddrEx = extaddrEx.decode()
                                
			ii += 1

			pid = struct.unpack("I", self.recvDatas[count][ii : ii + 4])[0]
			ii += 4
			
			cpu = struct.unpack("f", self.recvDatas[count][ii : ii + 4])[0]
			ii += 4

			mem = struct.unpack("f", self.recvDatas[count][ii : ii + 4])[0]
			ii += 4

			usedmem = struct.unpack("I", self.recvDatas[count][ii : ii + 4])[0]
			ii += 4

			state = struct.unpack("b", self.recvDatas[count][ii : ii + 1])[0]
			ii += 1
			
			machineID = struct.unpack("I", self.recvDatas[count][ii : ii + 4])[0]
			ii += 4
			
			extradata = struct.unpack("Q", self.recvDatas[count][ii : ii + 8])[0]
			ii += 8

			extradata1 = struct.unpack("Q", self.recvDatas[count][ii : ii + 8])[0]
			ii += 8
			
			extradata2 = struct.unpack("Q", self.recvDatas[count][ii : ii + 8])[0]
			ii += 8

			extradata3 = struct.unpack("Q", self.recvDatas[count][ii : ii + 8])[0]
			ii += 8
			
			backaddr = struct.unpack("I", self.recvDatas[count][ii : ii + 4])[0]
			ii += 4

			backport = struct.unpack("H", self.recvDatas[count][ii : ii + 2])[0]
			ii += 2
			
			#print("%s, uid=%i, cID=%i, gid=%i, groupid=%i, uname=%s" % (COMPONENT_NAME[componentType], \
			#	uid, componentID, globalorderid, grouporderid, username))
			
			componentInfos = self._interfaces.get(componentType)
			if componentInfos is None:
				componentInfos = []
				self._interfaces[componentType] = componentInfos
			
			found = False
			for info in componentInfos:
				if info[1] == componentID and info[13] == pid:
					found = True
					break
			
			if not found:
				componentInfos.append((uid, componentID, globalorderid, grouporderid, username, cpu, mem, usedmem, 0, \
									intaddr, intport, extaddr, extport, pid, machineID, state, componentType, extradata, extradata1, extradata2, extradata3, extaddrEx))
				
			count += 1
		
		self._interfaces_groups = {}
		self._interfaces_groups_uid = {}
		for ctype in self._interfaces:
			infos = self._interfaces.get(ctype, [])
			for info in infos:
				machineID = info[14]
				
				gourps = self._interfaces_groups.get(machineID, [])
				if machineID not in self._interfaces_groups:
					self._interfaces_groups[machineID] = gourps
					self._interfaces_groups_uid[machineID] = []
					
				if info[13] != machineID:
					gourps.append(info)
					if info[0] not in self._interfaces_groups_uid[machineID]:
						self._interfaces_groups_uid[machineID].append(info[0])
				else:
					gourps.insert(0, info)
				
class ClusterConsoleHandler(ClusterControllerHandler):
	def __init__(self, uid, consoleType):
		ClusterControllerHandler.__init__(self)
		self.uid = uid
		self.consoleType = consoleType
		
	def do(self):
		self._interfaces_groups = {}
		print("finding(" + self.consoleType  + ")...")
		self.queryAllInterfaces()
		interfaces = self._interfaces
		
		for machineID in self._interfaces_groups:
			infos = self._interfaces_groups.get(machineID, [])
			if len(infos) > 0:
				info = infos.pop(0)

			for info in infos:
				if COMPONENT_NAME[info[16]] + str(info[3]) == self.consoleType or \
					COMPONENT_NAME[info[16]] + str("%02d" %(info[3])) == self.consoleType:
					os.system('telnet %s %i' % (socket.inet_ntoa(struct.pack('I', info[9])), info[20]))
					return
		
		print("not found " + self.consoleType  + "!")
		
class ClusterQueryHandler(ClusterControllerHandler):
	def __init__(self, uid):
		ClusterControllerHandler.__init__(self)
		self.uid = uid
		
	def do(self):
		self._interfaces_groups = {}
		self.queryAllInterfaces()
		interfaces = self._interfaces
		
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
					(COMPONENT_NAME[info[16]], info[5], info[6], info[19] / 100.0, info[7] / 1024.0 / 1024.0, info[18] / 1024.0 / 1024.0, info[17] / 1024.0 / 1024.0, \
					socket.inet_ntoa(struct.pack('I', info[9]))))
			
			numComponent += len(infos)
			
			print("      proc\t\tcid\t\tuid\tpid\tgid\t%CPU\t%MEM\tusedMem\textra1\t\textra2\t\textra3")
			for info in infos:
				if info[16] == BASEAPP_TYPE:
					print("|-%12s%i\t%i\t%i\t%i\t%i\t%.2f\t%.2f\t%.2fm\tbases=%i\t\tclients=%i\tproxices=%i" % \
					(COMPONENT_NAME[info[16]], info[3], info[1], info[0], info[13], info[2], info[5], info[6], info[7] / 1024.0 / 1024.0, \
					info[17], info[18], info[19]))

					numBases += info[17]
					numClients += info[18]
					numProxices += info[19]
		
				elif info[16] == CELLAPP_TYPE:
					print("|-%12s%i\t%i\t%i\t%i\t%i\t%.2f\t%.2f\t%.2fm\tentities=%i\tcells=%i\t\t%i" % \
					(COMPONENT_NAME[info[16]], info[3], info[1], info[0],info[13], info[2], info[5], info[6], info[7] / 1024.0 / 1024.0, \
					info[17], info[18], 0))
					
					numEntities += info[17]
				else:
					print("|-%12s\t%i\t%i\t%i\t%i\t%.2f\t%.2f\t%.2fm\t%i\t\t%i\t\t%i" % \
					(COMPONENT_NAME[info[16]], info[1], info[0], info[13], info[2], info[5], info[6], info[7] / 1024.0 / 1024.0, \
					0, 0, 0))
					
			"""
			for info in infos:
				if info[16] == BASEAPP_TYPE:
					print("\t%s[%i]: uid=%i, pid=%i, gid=%i, tid=%i, %%CPU:%.2f, %%MEM:%.2f, usedMem=%.2fMB, entities=%i, clients=%i, addr=%s:%i" % \
					(COMPONENT_NAME[info[16]], info[1], self.uid, info[13], info[2], info[3], info[5], info[6], info[7] / 1024.0 / 1024.0, \
					info[17], info[18], socket.inet_ntoa(struct.pack('I', info[9])), socket.htons(info[10])))
				elif info[16] == CELLAPP_TYPE:
					print("\t%s[%i]: uid=%i, pid=%i, gid=%i, tid=%i, %%CPU:%.2f, %%MEM:%.2f, usedMem=%.2fMB, entities=%i, cells=%i, addr=%s:%i" % \
					(COMPONENT_NAME[info[16]], info[1], self.uid, info[13], info[2], info[3], info[5], info[6], info[7] / 1024.0 / 1024.0, \
					info[17], info[18], socket.inet_ntoa(struct.pack('I', info[9])), socket.htons(info[10])))
				else:
					print("\t%s[%i]: uid=%i, pid=%i, gid=%i, tid=%i, %%CPU:%.2f, %%MEM:%.2f, usedMem=%.2fMB, addr=%s:%i" % \
					(COMPONENT_NAME[info[16]], info[1], self.uid, info[13], info[2], info[3], info[5], info[6], info[7] / 1024.0 / 1024.0, \
					socket.inet_ntoa(struct.pack('I', info[9])), socket.htons(info[10])))
			"""
			
		print('-----------------------------------------------------')
		print("machines: %i, components=%i, numBases=%i, numProxices=%i, numClients=%i, numEntities=%i, numCells=%i." % \
			(len(self._interfaces_groups), numComponent, numBases, numProxices, numClients, numEntities, numCells))
		
class ClusterStartHandler(ClusterControllerHandler):
	def __init__(self, uid, startTemplate):
		ClusterControllerHandler.__init__(self)
		
		self.uid = uid
		self.startTemplate = startTemplate.split("|")
		
	def do(self):
		self.queryAllInterfaces()
		interfaces = self._interfaces
		
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
			self.writePacket("H", 10)
			self.writePacket("i", self.uid)
			self.writePacket("i", COMPONENT_NAME2TYPE[ctype])
			self.sendto()
		
		
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
				if info[0] == self.uid:
					clist.append(info[1])
						
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
					if info[0] == self.uid:
						clist.append(info[1])
						
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
				if info[0] == self.uid:
					clist.append(info[1])
			print("\t\t%s : %i\t%s" % (COMPONENT_NAME[ctype], len(clist), clist))
			
		print("ClusterStopHandler::do: completed!")

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
			templatestr = "dbmgr|baseappmgr|cellappmgr|baseapp|cellapp|loginapp"
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

			clusterHandler = ClusterStartHandler(uid, templatestr)
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
	else:
		uid = -1

		if len(sys.argv) >= 2:
			if sys.argv[1].isdigit():
				uid = sys.argv[1]

		uid = int(uid)
		if uid < 0:
			uid = getDefaultUID()

		clusterHandler = ClusterQueryHandler(uid)
			
	if clusterHandler is not None: 
		clusterHandler.do()
