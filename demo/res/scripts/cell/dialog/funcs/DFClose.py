# -*- coding: utf-8 -*-
#
"""
"""
from KBEDebug import *
from dialog.funcs.iDFunction import iDFunction

class DFClose(iDFunction):
	"""
	"""
	def __init__(self, args):
		pass
		
	def valid(self, avatar, args):
		return True

	def do(self, avatar, args):
		avatar.client.dialog_close()
		return True