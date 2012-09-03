# -*- coding: utf-8 -*-
#
"""
"""
from KBEDebug import *
from dialog.Dialog import Dialog
from dialog.DialogMgr import DialogMgr
import d_entities

g_dialogMgr = DialogMgr()

def onInit():
	"""
	init Dialog
	"""
	g_dialogMgr.onInit()
				
def onGossip(key, avatar, talker):
	INFO_MSG("onGossip: avatar:%i, talker=%i, key:%s." % (avatar.id, talker.id, key))
	
	if d_entities.datas.get(talker.utype, None) is None:
		return
		
	return g_dialogMgr.talk(key, avatar, talker, None)

