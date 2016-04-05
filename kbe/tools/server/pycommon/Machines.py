# -*- coding: utf-8 -*-

import socket, time, random
import sys, os, struct
import traceback
import select
import getpass

host = '' # Bind to all interfaces

MachineInterface_onFindInterfaceAddr = 1
MachineInterface_startserver = 2
MachineInterface_stopserver = 3
MachineInterface_onQueryAllInterfaceInfos = 4
MachineInterface_onQueryMachines = 5

from . import Define

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
		self.componentName = Define.COMPONENT_NAME[self.componentType]
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
		
		#print("%s, uid=%i, cID=%i, gid=%i, groupid=%i, uname=%s" % (Define.COMPONENT_NAME[self.componentType], \
		#	self.uid, self.componentID, self.globalOrderID, self.groupOrderID, self.username))
		

class Machines:
	def __init__(self, uid = None, username = None, listenPort = 0):
		"""
		"""
		self.udp_socket = None
		self.listenPort = listenPort
		
		if uid is None:
			uid = Define.getDefaultUID()
		
		if username is None:
			username = Define.pwd.getpwuid( uid ).pw_name
			
		self.uid = uid
		self.username = username
		if type(self.username) is str:
			self.username = username.encode( "utf-8" )
		
		self.startListen()
		
		self.reset()
		
	def __del__(self):
		#print( "Machines destroy now" )
		self.stopListen()
		
	def startListen(self):
		"""
		"""
		assert self.udp_socket is None
		self.udp_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
		self.udp_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
		self.udp_socket.bind((host, self.listenPort))
		self.replyPort = self.udp_socket.getsockname()[1]
		#print( "udp receive addr: %s" % (self.udp_socket.getsockname(), ) )
		
	def stopListen(self):
		"""
		"""
		self.udp_socket.close()
		self.udp_socket = None
		
	def reset(self):
		"""
		"""
		self.interfaces = {}
		self.interfaces_groups = {}
		self.interfaces_groups_uid = {}
		self.machines = []
		
	def send(self, msg, ip = "<broadcast>", trycount = 1, timeout = 1, callback = None):
		"""
		发送消息
		"""
		_udp_broadcast_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
		_udp_broadcast_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
		
		if ip == "<broadcast>":
			_udp_broadcast_socket.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
			_udp_broadcast_socket.sendto(msg, ('255.255.255.255', 20086))
		else:
			_udp_broadcast_socket.sendto(msg, (ip, 20086))
		
	def sendAndReceive(self, msg, ip = "<broadcast>", trycount = 1, timeout = 1, callback = None):
		"""
		发送消息，并等待消息返回
		"""
		self.send(msg, ip, trycount, timeout, callback)
		
		self.udp_socket.settimeout(timeout)
		dectrycount = trycount
		
		recvDatas = []
		while(dectrycount > 0):
			try:
				datas, address = self.udp_socket.recvfrom(10240)
				recvDatas.append(datas)
				#print ("%s received %s data from %r" % (len(recvDatas), len(datas), address))
				if callable( callback ):
					try:
						if callback( datas, address ):
							return recvDatas
					except:
						traceback.print_exc()
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
		self.reset()
		nameLen = len( self.username ) + 1 # 加1是为了存放空终结符
		
		msg = Define.BytesIO()
		msg.write( struct.pack("=H", MachineInterface_onQueryAllInterfaceInfos ) ) # command
		msg.write( struct.pack("=H", struct.calcsize("=iH") + nameLen ) ) # command length
		msg.write( struct.pack("=i", self.uid) )
		msg.write( struct.pack("=%ss" % nameLen, self.username) )
		msg.write( struct.pack("=H", socket.htons(self.replyPort)) ) # reply port

		datas = self.sendAndReceive( msg.getvalue(), ip, trycount, timeout )
		self.parseQueryDatas( datas )

	def queryMachines(self, ip = "<broadcast>", trycount = 1, timeout = 1):
		"""
		"""
		self.reset()
		nameLen = len( self.username ) + 1 # 加1是为了产生空终结符
		
		msg = Define.BytesIO()
		msg.write( struct.pack("=H", MachineInterface_onQueryMachines ) ) # command
		msg.write( struct.pack("=H", struct.calcsize("=iH") + nameLen ) ) # command length
		msg.write( struct.pack("=i", self.uid) )
		msg.write( struct.pack("=%ss" % nameLen, self.username) )
		msg.write( struct.pack("=H", socket.htons(self.replyPort)) ) # reply port

		datas = self.sendAndReceive( msg.getvalue(), ip, trycount, timeout )
		self.parseQueryDatas( datas )

	def startServer(self, componentType, cid, gus, targetIP):
		"""
		"""
		msg = Define.BytesIO()
		msg.write( struct.pack("=H", MachineInterface_startserver ) ) # command
		msg.write( struct.pack("=H", struct.calcsize("=iiQhH") ) ) # command length
		msg.write( struct.pack("=i", self.uid) )
		msg.write( struct.pack("=i", componentType) )
		msg.write( struct.pack("=Q", cid) )
		msg.write( struct.pack("=h", gus) )
		msg.write( struct.pack("=H", socket.htons(self.replyPort)) ) # reply port
		self.sendAndReceive( msg.getvalue(), targetIP )

	def stopServer(self, componentType, targetIP = "<broadcast>"):
		"""
		"""
		msg = Define.BytesIO()
		msg.write( struct.pack("=H", MachineInterface_stopserver ) ) # command
		msg.write( struct.pack("=H", struct.calcsize("=iiH") ) ) # command length
		msg.write( struct.pack("=i", self.uid) )
		msg.write( struct.pack("=i", componentType) )
		msg.write( struct.pack("=H", socket.htons(self.replyPort)) ) # reply port
		self.sendAndReceive( msg.getvalue(), targetIP )

	def parseQueryDatas( self, recvDatas ):
		"""
		"""
		for data in recvDatas:
			self.parseQueryData( data )

	def parseQueryData( self, recvData ):
		"""
		"""
		cinfo = ComponentInfo( recvData )

		componentInfos = self.interfaces.get(cinfo.componentType)
		if componentInfos is None:
			componentInfos = []
			self.interfaces[cinfo.componentType] = componentInfos
		
		found = False
		for info in componentInfos:
			if info.componentID == cinfo.componentID and info.pid == cinfo.pid:
				found = True
				break
		
		if found:
			return
			
		componentInfos.append(cinfo)
		
		machineID = cinfo.machineID
		
		gourps = self.interfaces_groups.get(machineID, [])
		if machineID not in self.interfaces_groups:
			self.interfaces_groups[machineID] = gourps
			self.interfaces_groups_uid[machineID] = []
			
		# 如果pid与machineID相等，说明这个是machine进程
		if cinfo.pid != machineID:
			gourps.append(cinfo)
			if cinfo.uid not in self.interfaces_groups_uid[machineID]:
				self.interfaces_groups_uid[machineID].append(cinfo.uid)
		else:
			# 是machine进程，把它放在最前面，并且加到machines列表中
			gourps.insert(0, cinfo)
			self.machines.append( cinfo )

	def makeGUS(self, componentType):
		"""
		生成一个相对唯一的gus（非全局唯一）
		"""
		if not hasattr( self, "ct2gus" ):
			self.ct2gus = [0] * Define.COMPONENT_END_TYPE
		
		self.ct2gus[componentType] += 1
		return componentType * 100 + self.ct2gus[componentType]
	
	def makeCID(self, componentType):
		"""
		生成相对唯一的cid（非全局唯一）
		"""
		if not hasattr( self, "cidRand" ):
			self.cidRand = random.randint(1, 99999)

		if not hasattr( self, "ct2gus" ):
			self.ct2cid = [0] * Define.COMPONENT_END_TYPE

		self.ct2cid[componentType] += 1
		t = int( time.time() ) % 99999
		cid = "%02i%05i%05i%04i" % (componentType, t, self.cidRand, self.ct2cid[componentType])
		return int(cid)
	
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

