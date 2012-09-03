# -*- coding: utf-8 -*-
#
"""
"""
from KBEDebug import *
from dialog.funcs.iDFunction import iDFunction

class DFTeleport(iDFunction):
	"""
	"""
	def __init__(self, args):
		self.spaceType = int(args[0]) # 地图ID
		self.position = (float(args[1]), float(args[2]), float(args[3]))

	def valid(self, avatar, args):
		return True

	def do(self, avatar, args):
		return True