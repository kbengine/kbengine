# -*- coding: gb2312 -*-
#
#-----------------------------------------------------------------------------------------------------------------------------------
# 天赋
def funcGeniusOver(mapDict, allDatas, datas, dataName):
	"""
	"""
	if dataName != "datas":
		return datas
	d = {}
	for value in datas.values():
		raceclass = value["raceclass"]
		if raceclass not in d:
			d[raceclass] = {}

		dd = d[raceclass]
		geniusClass = value["genuisClass"]

		if geniusClass not in dd:
			dd[geniusClass] = {}

		vv = []
		for x in value["effs"]:
			v = x.split("`")
			vv.append([int(e) for e in v])

		value["effs"] = vv
		dd[geniusClass][value["ID"]] = value

	return d

def funcGeniusDefs(datas):
	return ""

def funcGeniusGlobalDefs(datas):
	e = r'''
import scdefine
from bwdebug import *
from data_common.upgradeDatas import datas as d_upgradeDatas

def checkConfigure(avatar, geniusclass, gid, isInc):
	"""
	@param geniusClass	: scdefine.GENIUS_TYPE_*
	"""
	geniusDatas = datas.get(avatar.getPro(), {}).get(geniusclass, {}).get(gid)
	if geniusDatas is None:
		ERROR_MSG("%i not found gid=%i" % (avatar.id, gid))
		return False

	if geniusDatas["point"] <= 0:
		ERROR_MSG("%i gid=%i is rewarded." % (avatar.id, gid))
		return False

	if isInc:
		totalPoint = avatar.geniusInfos.getTotalPoint()
		enableGeniusPoint = avatar.getActiveGeniusPoint()

		if totalPoint >= enableGeniusPoint:
			DEBUG_MSG("%i currTotalPoint=%i >= enableGeniusPoint=%i" % (avatar.id, totalPoint, enableGeniusPoint))
			return False

		needPoint = geniusDatas["needPoint"]
		if totalPoint < needPoint:
			DEBUG_MSG("%i currTotalPoint=%i < needPoint=%i" % (avatar.id, totalPoint, needPoint))
			return False

		needID = geniusDatas["needID"]
		if needID > 0 and needID not in avatar.geniusInfos:
			DEBUG_MSG("%i need genius=%i" % (avatar.id, needID))
			return False
	else:
		v = avatar.geniusInfos.get(gid, 0)
		if v <= 0:
			DEBUG_MSG("%i geniusPoint[%i] == %i" % (avatar.id, gid, v))
			return False

	return True

def getGeniusData(avatarPro, geniusclass):
	"""
	@param avatarPro	: 职业 avatar.getPro()
	@param geniusClass	: scdefine.GENIUS_TYPE_*
	"""
	return datas.get(avatarPro, {}).get(geniusclass, {})

def getGeniusDataByGID(gid):
	for pro, values in datas.items():
		for geniusClass, infos in values.items():
			if gid in infos:
				return infos[gid]
	return None

def getRewardGenius(avatarPro, geniusclass):
	"""
	@param avatarPro	: 职业 avatar.getPro()
	@param geniusClass	: scdefine.GENIUS_TYPE_*
	"""
	ret = {}
	geniusDatas = getGeniusData(avatarPro, geniusclass)
	if geniusDatas is None:
		return ret

	for gid, value in geniusDatas.items():
		layer = value["layer"]
		if layer not in ret:
			ret[layer] = []

		if value["point"] <= 0:
			ret[layer].append(gid)

	return ret

def getGeniusClassByGID(gid):
	for pro, values in datas.items():
		for geniusClass, infos in values.items():
			if gid in infos:
				return geniusClass
	return scdefine.GENIUS_TYPE_UNKNOWN
	'''
	return e

#-----------------------------------------------------------------------------------------------------------------------------------
# 初始化表
def funcAvatarInitDatasOver(mapDict, allDatas, datas, dataName):
	d = {}
	keys = ("equipBoxItemIDs", "commonBoxItemIDs",)

	for value in datas.values():
		race = value["race"]
		pro = value["pro"]
		gender = value["gender"]

		t = race | pro | gender
		value["id"] = t

		for key in keys:
			vvv = []
			for v in value[key]:
				vv = v.split(":")
				amount = 1
				itemID = int(vv[0])
				if len(vv) > 1:
					amount = int(vv[1])
				vvv.append((itemID, amount))
			value[key] = vvv

		d[t] = value
	return d

#-----------------------------------------------------------------------------------------------------------------------------------
# buff状态表
def funcBuffStateOver(mapDict, allDatas, datas, dataName):
	"""
	"""
	d = {}
	for value in datas.values():
		id = value["id"]
	return datas

#-----------------------------------------------------------------------------------------------------------------------------------
# 球应用表
def funcPowerDefinesOver(mapDict, allDatas, datas, dataName):
	"""
	"""
	d = {}
	for value in datas.values():
		pro = value["pro"]
		if pro not in d:
			d[pro] = {}

		dd = d[pro]
		geniusClass = value["genius"]
		if geniusClass not in dd:
			dd[geniusClass] = {}

		del value["id"]
		del value["genius"]
		del value["pro"]
		dd[geniusClass] = value

	return d

#-----------------------------------------------------------------------------------------------------------------------------------
def funcLevelDiffGlobalDefs(datas):
	e = r'''
def getLimit():
	keys = datas.keys()
	keys.sort()
	return (keys[0], keys[-1])
	'''
	return e 
	
#-----------------------------------------------------------------------------------------------------------------------------------
# 角色基础属性
def funcAvatarBaseAttrDatasOver(mapDict, allDatas, datas, dataName):
	d = {}
	for value in datas.values():
		pro = value["pro"]
		try:
			proDatas = d[pro]
		except:
			proDatas = {}
			d[pro] = proDatas

		proDatas[value["level"]] = value
		del value["level"]
		del value["id"]
	return d