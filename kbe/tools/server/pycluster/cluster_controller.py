import socket
import sys
import os
import traceback

host = '' # Bind to all interfaces
port = 1234

class ClusterControllerHandler:
	def __init__(self):
		self._udp_broadcast_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
		self._udp_broadcast_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
		self._udp_broadcast_socket.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
		
		self._udp_broadcast_socket.bind((host, port))
		
		self._tcp_socket = socket.socket()
		
	def do(self):
		self._udp_broadcast_socket.sendto(b"server here", ('255.255.255.255', 20086))
		print("posted udp broadcast, waiting for recv...")
		
		try:
		    data, address = self._udp_broadcast_socket.recvfrom(2345)
		    print ("get data form", address, ":", data)
		    print ("received %r from %r" % (data, address))
		except (KeyboardInterrupt, SystemExit):
		    raise
		except:
		    traceback.print_exc()
        
class ClusterConsoleHandler(ClusterControllerHandler):
	def __init__(self, consoleType):
		ClusterControllerHandler.__init__(self)
		self.consoleType = consoleType
		
	def do(self):
		print ("ClusterConsoleHandler::do: console %s" % self.consoleType)
		ClusterControllerHandler.do(self)

class ClusterStartHandler(ClusterControllerHandler):
	def __init__(self, uid):
		ClusterControllerHandler.__init__(self)
		self.uid = uid
		
	def do(self):
		print ("ClusterStartHandler::do: start uid=%s" % self.uid)
		ClusterControllerHandler.do(self)
		
class ClusterStopHandler(ClusterControllerHandler):
	def __init__(self, uid):
		ClusterControllerHandler.__init__(self)
		self.uid = uid
		
	def do(self):
		print ("ClusterStopHandler::do: stop uid=%s" % self.uid)
		ClusterControllerHandler.do(self)

if __name__ == "__main__":
	clusterHandler = None
	
	if len(sys.argv)  > 1:
		cmdType = sys.argv[1]
		if cmdType == "start":
			if len(sys.argv) == 3:
				uid = sys.argv[2]
			else:
				uid = os.environ.get('uid')
			
			uid = int(uid)
			assert(uid > 0)
			clusterHandler = ClusterStartHandler(uid)
		elif cmdType == "stop":
			if len(sys.argv) == 3:
				uid = sys.argv[2]
			else:
				uid = os.environ.get('uid')
			
			uid = int(uid)
			assert(uid > 0)
			clusterHandler = ClusterStopHandler(uid)
		elif cmdType == "console":
			assert(len(sys.argv) == 3)
			consoleType = sys.argv[2]
			clusterHandler = ClusterConsoleHandler(consoleType)
			
	if clusterHandler is not None:
		clusterHandler.do()