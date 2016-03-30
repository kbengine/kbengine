# -*- coding: utf-8 -*-

import socket, io, time
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
MachineInterface_onQueryMachines = 5

#from Define import *


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
		

class Machines:
	def __init__(self, uid, username):
		"""
		"""
		self.uid = uid
		self.username = username
		if type(username) is str:
			self.username = username.encode( "utf-8" )
		else:
			self.username = username
		
		
		self.udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
		self.udp_socket.bind((host, 0))
		
		self.interfaces = {}
		self.interfaces_groups = {}
		self.interfaces_groups_uid = {}
		self.machines = []
		
	def sendAndReceive(self, msg, ip = "<broadcast>", trycount = 1, timeout = 1):
		"""
		"""
		_udp_broadcast_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
		_udp_broadcast_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
		
		if ip == "<broadcast>":
			_udp_broadcast_socket.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
			_udp_broadcast_socket.sendto(msg, ('255.255.255.255', 20086))
		else:
			_udp_broadcast_socket.sendto(msg, (ip, 20086))
		
		self.udp_socket.settimeout(timeout)
		dectrycount = trycount
		
		recvDatas = []
		while(dectrycount > 0):
			try:
				dectrycount = trycount
				datas, address = self.udp_socket.recvfrom(10240)
				recvDatas.append(datas)
				#print ("received %s data from %r" % (len(recvDatas), address))
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
		return recvDatas

	def queryAllInterfaces(self, ip = "<broadcast>", trycount = 1, timeout = 1):
		"""
		"""
		nameLen = len( self.username ) + 1 # 加1是为了存放空终结符
		
		msg = io.BytesIO()
		msg.write( struct.pack("=H", MachineInterface_onQueryAllInterfaceInfos ) ) # command
		msg.write( struct.pack("=H", struct.calcsize("=iH") + nameLen ) ) # command length
		msg.write( struct.pack("=i", self.uid) )
		msg.write( struct.pack("=%ss" % nameLen, self.username) )
		msg.write( struct.pack("=H", socket.htons(self.udp_socket.getsockname()[1])) )

		datas = self.sendAndReceive( msg.getvalue(), ip, trycount, timeout )
		self.parseQueryDatas( datas )

	def queryMachines(self, ip = "<broadcast>", trycount = 1, timeout = 1):
		"""
		"""
		nameLen = len( self.username ) + 1 # 加1是为了产生空终结符
		
		msg = io.BytesIO()
		msg.write( struct.pack("=H", MachineInterface_onQueryMachines ) ) # command
		msg.write( struct.pack("=H", struct.calcsize("=iH") + nameLen ) ) # command length
		msg.write( struct.pack("=i", self.uid) )
		msg.write( struct.pack("=%ss" % nameLen, self.username) )
		msg.write( struct.pack("=H", socket.htons(self.udp_socket.getsockname()[1])) )

		datas = self.sendAndReceive( msg.getvalue(), ip, trycount, timeout )
		self.parseQueryDatas( datas )

	def parseQueryDatas( self, recvDatas ):
		self.interfaces = {}
		if len(recvDatas) == 0:
			return
		
		count = 0

		while(count < len(recvDatas)):
			cinfo = ComponentInfo( recvDatas[count] )
			count += 1

			componentInfos = self.interfaces.get(cinfo.componentType)
			if componentInfos is None:
				componentInfos = []
				self.interfaces[cinfo.componentType] = componentInfos
			
			found = False
			for info in componentInfos:
				if info.componentID == cinfo.componentID and info.pid == cinfo.pid:
					found = True
					break
			
			if not found:
				componentInfos.append(cinfo)
		
		self.interfaces_groups = {}
		self.interfaces_groups_uid = {}
		self.machines = []
		for ctype in self.interfaces:
			infos = self.interfaces.get(ctype, [])
			for info in infos:
				machineID = info.machineID
				
				gourps = self.interfaces_groups.get(machineID, [])
				if machineID not in self.interfaces_groups:
					self.interfaces_groups[machineID] = gourps
					self.interfaces_groups_uid[machineID] = []
					
				# 如果pid与machineID相等，说明这个是machine进程
				if info.pid != machineID:
					gourps.append(info)
					if info.uid not in self.interfaces_groups_uid[machineID]:
						self.interfaces_groups_uid[machineID].append(info.uid)
				else:
					# 是machine进程，把它放在最前面，并且加到machines列表中
					gourps.insert(0, info)
					self.machines.append( info )

	def getMachine( self, ip ):
		"""
		通过ip地址找到对应的machine的info
		"""
		for info in self.machines:
			if info.intaddr == ip:
				return info
		return None
	
	def getComponentInfos( self, componentType ):
		"""
		获取某一类型的组件信息
		"""
		return self.interfaces.get( componentType, [] )

