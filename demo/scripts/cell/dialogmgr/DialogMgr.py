# -*- coding: utf-8 -*-
#
"""
"""
from KBEDebug import *
from dialogmgr.Dialog import Dialog
from d_dialogs import datas as g_dialogDatas

class DialogMgr:
	"""
	"""
	def __init__(self):
		self.__dialogs = {}
		Dialog.c_dialogMgr = self
		
	def onInit(self):
		"""
		"""
		for key, datas in g_dialogDatas.items():
			self.__dialogs[key] = Dialog(datas)
			
	def getDialog(self, key):
		return self.__dialogs[key]

	def talk(self, key, avatar, talker, args):
		dialog = self.__dialogs.get(key)
		if dialog and dialog.canTalk(avatar, talker):
			dialog.onTalk(avatar, talker)
