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
CENTER_TYPE				= 9
CONSOLE_TYPE			= 10
MESSAGELOG_TYPE			= 11
RESOURCEMGR_TYPE		= 12
BOTS_TYPE				= 13
WATCHER_TYPE			= 14
BILLING_TYPE			= 15
COMPONENT_END_TYPE		= 16

# ת
COMPONENT_NAME2TYPE = {
	"unknown"		: UNKNOWN_COMPONENT_TYPE,
	"dbmgr"			: DBMGR_TYPE,
	"loginapp"		: LOGINAPP_TYPE,
	"baseappmgr"	: BASEAPPMGR_TYPE,
	"cellappmgr"	: CELLAPPMGR_TYPE,
	"cellapp" 		: CELLAPP_TYPE,
	"baseapp" 		: BASEAPP_TYPE,
	"client" 		: CLIENT_TYPE,
	"kbmachine"		: MACHINE_TYPE,
	"kbcenter" 		: CENTER_TYPE,
	"console" 		: CONSOLE_TYPE,
	"messagelog" 	: MESSAGELOG_TYPE,
	"resourcemgr"	: RESOURCEMGR_TYPE,
	"bots" 			: BOTS_TYPE,
	"watcher" 		: WATCHER_TYPE,
	"billing" 		: BILLING_TYPE,
	"billingsystem" : BILLING_TYPE
}

#  
COMPONENT_NAME = (
	"unknown",
	"dbmgr",
	"loginapp",
	"baseappmgr",
	"cellappmgr",
	"cellapp",
	"baseapp",
	"client",
	"kbmachine",
	"kbcenter",
	"console",
	"messagelog",
	"resourcemgr",
	"bots",
	"watcher",
	"billing",
)



class ClusterControllerHandler:
	def __init__(self):
		self.udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
		self.udp_socket.bind((host, 0))
		
		self._tcp_socket = socket.socket()
		self.resetPacket()
		
		self._interfaces = {}
		
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
				recvdata, address = self.udp_socket.recvfrom(4096)
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
				if x == 0:
					break
					
				ii += 1
			
			username = self.recvDatas[count][i: ii].decode()
			ii += 1
			
			componentType = struct.unpack("B", self.recvDatas[count][ii : ii + 1])[0]
			ii += 1
			
			componentID = struct.unpack("Q", self.recvDatas[count][ii : ii + 8])[0]
			ii += 16
			
			globalorderid = struct.unpack("B", self.recvDatas[count][ii : ii + 1])[0]
			ii += 1
		
			grouporderid = struct.unpack("B", self.recvDatas[count][ii : ii + 1])[0]
			ii += 1

			intaddr = struct.unpack("I", self.recvDatas[count][ii : ii + 4])[0]
			ii += 4

			intport = struct.unpack("H", self.recvDatas[count][ii : ii + 2])[0]
			ii += 2

			extaddr = struct.unpack("I", self.recvDatas[count][ii : ii + 4])[0]
			ii += 4

			extport = struct.unpack("H", self.recvDatas[count][ii : ii + 2])[0]
			ii += 2
			
			#print("%s, uid=%i, cID=%i, gid=%i, groupid=%i, uname=%s" % (COMPONENT_NAME[componentType], \
			#	uid, componentID, globalorderid, grouporderid, username))
			
			componentInfos = self._interfaces.get(componentType)
			if componentInfos is None:
				componentInfos = []
				self._interfaces[componentType] = componentInfos
			
			componentInfos.append((uid, componentID, globalorderid, grouporderid, username))
			
			count += 1
			
class ClusterConsoleHandler(ClusterControllerHandler):
	def __init__(self, consoleType, startTemplate):
		ClusterControllerHandler.__init__(self)
		self.consoleType = consoleType
		self.startTemplate = startTemplate.split("|")
		
	def do(self):
		for ctype in self.startTemplate:
			print ("ClusterConsoleHandler::do: find uid=%s, type=%s" % (self.uid, ctype))
			
			if ctype not in COMPONENT_NAME2TYPE:
				print("not found %s!" % ctype)
				continue
				
			self.writePacket("H", MachineInterface_onFindInterfaceAddr)
			self.writePacket("H", 10)
			self.writePacket("i", self.uid)
			self.writePacket("i", COMPONENT_NAME2TYPE[ctype])
			self.sendto()

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
			
	def do(self):
		self.queryAllInterfaces()
		interfaces = self._interfaces
		
		if len(self.startTemplate) <= 0:
			for ctype in self._interfaces:
				infos = self._interfaces.get(ctype, [])
				for x in range(0, len(infos)):
					self.startTemplate.append(COMPONENT_NAME[ctype])

		interfacesCount = {}
		interfacesCount1 = {}
		
		print("online-components:")
		printed = []
		for ctype in self.startTemplate:
			if ctype not in COMPONENT_NAME2TYPE:
				print("not found %s, stop failed!" % ctype)
				continue
			
			infos = interfaces.get(COMPONENT_NAME2TYPE[ctype], [])
			interfacesCount[ctype] = len(infos)

			if ctype in interfacesCount1:
				interfacesCount1[ctype] += 1
			else:
				interfacesCount1[ctype] = 1
			
			if interfacesCount1[ctype] > interfacesCount[ctype]:
				continue
			
			if ctype not in printed:
				printed.append(ctype)
				print("\t\t%s : %i" % (ctype, len(infos)))
				
			self.writePacket("H", MachineInterface_stopserver)
			self.writePacket("H", 10)
			self.writePacket("i", self.uid)
			self.writePacket("i", COMPONENT_NAME2TYPE[ctype])
			self.sendto()
			
			#print ("ClusterStopHandler::do: stop uid=%s, type=%s, send=%s" % (self.uid, ctype, \
			#	len(self.recvDatas) > 0 and self.recvDatas[0] == b'\x01'))
			
		qcount = 1
		while(True):
			print("query status: %i" % qcount)
			qcount += 1
			
			self.queryAllInterfaces()
			
			waitcount = 0
			for ctype in interfacesCount:
				if ctype not in COMPONENT_NAME2TYPE:
					continue
			
				infos = self._interfaces.get(COMPONENT_NAME2TYPE[ctype], [])
				print("\t\t%s : %i" % (ctype, len(infos)))
				waitcount += len(infos)
			
			if waitcount > 0:
				time.sleep(3)
			else:
				break
		
		print("[other-online-components:]")
		for ctype in self._interfaces:
			infos = self._interfaces.get(ctype, [])
			print("\t\t%s : %i" % (COMPONENT_NAME[ctype], len(infos)))
			
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
	
	return uid
	
if __name__ == "__main__":
	clusterHandler = None
	
	if len(sys.argv)  > 1:
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
		elif cmdType == "console":
			assert(len(sys.argv) == 3)
			consoleType = sys.argv[2]
			clusterHandler = ClusterConsoleHandler(consoleType)
			
	if clusterHandler is not None: 
		clusterHandler.do()