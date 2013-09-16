# -*- coding: utf-8 -*-
#
"""
"""
from KBEDebug import *
from dialogmgr.funcs.iDFunction import iDFunction

class DFTeleport(iDFunction):
	"""
	"""
	def __init__(self, args):
		args = args.split(",")
		self.spaceType = int(args[0]) # 地图ID
		
		if '0' == args[1]:
			param1 = (0,0,0)
		else:
			param1 = args[1].split("`")
			param1 = (int(param1[0]), 0, int(param1[1]))
			
		self.position = param1
		self.yaw = int(args[2])
		
	def valid(self, avatar, args):
		return True

	def do(self, avatar, args):
		avatar.teleportSpace(self.spaceType, tuple(self.position), (0, 0, self.yaw), {})
		return True