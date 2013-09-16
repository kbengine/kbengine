# -*- coding: utf-8 -*-
#
"""
"""
import KBEngine
import copy
import scdefine
import scutils
import skills
import ScriptMaps
from KBEDebug import *
from Item import Item
from Equip import Equip
from Weapon import Weapon
from d_items import datas as d_items

def onInit():
	pass

def checkItemNo(itemNO):
	return itemNO in d_items

def noAlias2ItemNo(aid):
	return g_noAlias2ItemNo.get(aid, 0)

def itemNo2NoAlias(sid):
	return g_itemNo2NoAlias.get(sid, 0)

def getItemData(itemNO):
	"""
	获得物品的配置数据
	"""
	return d_items.get(itemNO, {})

def getItemClass(itemNO):
	"""
	获得物品的配置数据
	"""
	return d_items[itemNO]["script"]

def createItem(itemNO, amount = 1, owner = None):
	"""
	创建物品
	"""
	INFO_MSG("%i created. amount=%i" % (itemNO, amount))

	stackMax = getItemData(itemNO).get("overlayMax", 1)
	itemList = []
	while amount > 0:
		itemAmount = (amount < stackMax) and amount or stackMax
		item = getItemClass(itemNO)(itemNO, scutils.newUID(), itemAmount)
		item.onCreate(owner)
		itemList.append(item)
		amount -= itemAmount

	return itemList

def createItemByItem(item, amount, owner = None):
	"""
	根据一个已知物品来创建一个新的物品
	"""
	newItem = copy.deepcopy(item)
	newItem.setUUID(scutils.newUID())
	newItem.setAmount(amount)
	INFO_MSG("new item created. new item uuid=%i, src item uuid=%i" % (newItem.getUUID(), item.getUUID()))
	return newItem