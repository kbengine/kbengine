# -*- coding: utf-8 -*-

import socket
import sys
import os
import struct
import traceback
import select

host = '' # Bind to all interfaces
port = 1234

MachineInterface_onFindInterfaceAddr = 1
MachineInterface_startserver = 2
MachineInterface_stopserver = 3

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
		self._udp_broadcast_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
		self._udp_broadcast_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
		self._udp_broadcast_socket.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
		
		self._udp_broadcast_socket.bind((host, port))
		self._tcp_socket = socket.socket()
		self.resetPacket()
		
	def do(self):
		pass
	
	def resetPacket(self):
		self.postDatas = b""
		
	def sendto(self):
		self._udp_broadcast_socket.settimeout(5)
		self._udp_broadcast_socket.sendto(self.postDatas, ('255.255.255.255', 20086))
		print("posted udp broadcast(size=%i), waiting for recv..." % len(self.postDatas))
		self.resetPacket()
		
		try:
			data, address = self._udp_broadcast_socket.recvfrom(2345)
			print ("received %r from %r" % (data, address))
		except socket.timeout: 
			print ("recvfrom timeout!")
		except (KeyboardInterrupt, SystemExit):
			raise
		except:
			traceback.print_exc()

	def writePacket(self, fmt, data):
		self.postDatas += struct.pack(fmt, data)
     	 
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
			self.writePacket("H", 8)
			self.writePacket("i", self.uid)
			self.writePacket("i", COMPONENT_NAME2TYPE[ctype])
			self.sendto()

class ClusterStartHandler(ClusterControllerHandler):
	def __init__(self, uid, startTemplate):
		ClusterControllerHandler.__init__(self)
		
		self.uid = uid
		self.startTemplate = startTemplate.split("|")
		
	def do(self):
		for ctype in self.startTemplate:
			print ("ClusterStartHandler::do: start uid=%s, type=%s" % (self.uid, ctype))
			if ctype not in COMPONENT_NAME2TYPE:
				print("not found %s, start failed!" % ctype)
				continue
				
			self.writePacket("H", MachineInterface_startserver)
			self.writePacket("H", 8)
			self.writePacket("i", self.uid)
			self.writePacket("i", COMPONENT_NAME2TYPE[ctype])
			self.sendto()
		
class ClusterStopHandler(ClusterControllerHandler):
	def __init__(self, uid, startTemplate):
		ClusterControllerHandler.__init__(self)
		self.uid = uid
		self.startTemplate = startTemplate.split("|")
		
	def do(self):
		for ctype in self.startTemplate:
			print ("ClusterStopHandler::do: stop uid=%s, type=%s" % (self.uid, ctype))
			if ctype not in COMPONENT_NAME2TYPE:
				print("not found %s, stop failed!" % ctype)
				continue
				
			self.writePacket("H", MachineInterface_stopserver)
			self.writePacket("H", 8)
			self.writePacket("i", self.uid)
			self.writePacket("i", COMPONENT_NAME2TYPE[ctype])
			self.sendto()

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
				uid = int(os.environ.get('uid', 0))

			clusterHandler = ClusterStartHandler(uid, templatestr)
		elif cmdType == "stop":
			templatestr = "loginapp|baseappmgr|cellappmgr|cellapp|baseapp|dbmgr"
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
				uid = int(os.environ.get('uid', 0))

			clusterHandler = ClusterStopHandler(uid, templatestr)
		elif cmdType == "console":
			assert(len(sys.argv) == 3)
			consoleType = sys.argv[2]
			clusterHandler = ClusterConsoleHandler(consoleType)
			
	if clusterHandler is not None: 
		clusterHandler.do()