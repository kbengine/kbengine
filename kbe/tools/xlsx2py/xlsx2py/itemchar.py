# -*- coding: gb2312 -*-
#


#-----------------------------------------------------------------------------------------------------------------------------------
# 物品
def funcItemOver(mapDict, allDatas, datas, dataName):
	"""
	"""
	d = {}
	if dataName == "typeMap":
		for value in datas.values():
			typeNO = value["typeNO"]
			if typeNO not in d:
				d[typeNO] = {}

			dd = d[typeNO]
			subTypeNO = value["subTypeNO"]
			dd[subTypeNO] = value
	else:
		noAlias = 1
		for value in datas.values():
			# 增加物品ID的别名
			value["noAlias"] = noAlias
			noAlias += 1

			t = value["type"]
			if t == 1:
				value["equipTypeIndex"] = value["subType"]
			if t != 2 and t != 3 and t != 7:
				continue
			tt = value["subType"]
			typeMap = allDatas["typeMap"]
			rigType = typeMap[t][tt]["rigType"]
			value["rigType"] = rigType
		d = datas
	return d

# 掉落
def funcDropOver(mapDict, allDatas, datas, dataName):
	"""
	"""
	d = {}
	if dataName == "worldDatas":
		for dropID, dropGroups in datas.items():
			d[dropID] = {}
			for name, value in dropGroups.items():
				if name == "no" or value == ():
					continue

				d[dropID][name] = {"itemNo" : value[0],"itemNum" : value[1],"itemProb" : value[2],}

			if len(d[dropID]) == 0:
				del d[dropID]

	elif dataName == "specialDatas":
		for dropID, dropDatas in datas.items():
			d[dropID] = {}
			for bigGroupIdx in range(1, 6):
				smallGroups = {}
				for smallGroupIdx in range(1, 11):
					dropGroupKey = "%s_%d_%d" % ("dropGroup", bigGroupIdx, smallGroupIdx)
					if dropDatas[dropGroupKey] == ():
						continue
					conditionKey = "%s_%d_%d" % ("condition", bigGroupIdx, smallGroupIdx)
					conditionValueKey = "%s_%d_%d" % ("conditionValue", bigGroupIdx, smallGroupIdx)
					smallGroupsKey = "%s_%d" % ("dropSmallGroup", smallGroupIdx)
					smallGroups[smallGroupsKey] = {
													"itemNo" : dropDatas[dropGroupKey][0], "itemNum" : dropDatas[dropGroupKey][1],	\
													"itemProb" : dropDatas[dropGroupKey][2], "condition" : dropDatas[conditionKey],	\
													"conditionValue" : dropDatas[conditionValueKey],
													}
				if smallGroups == {}:
					continue

				d[dropID]["%s_%d" % ("dropBigGroup", bigGroupIdx)] = {}
				bigGroups = d[dropID]["%s_%d" % ("dropBigGroup", bigGroupIdx)]
				bigGroups["dropNum"] = dropDatas["%s_%d" % ("dropNum", bigGroupIdx)]
				bigGroups["dropSmallGroups"] = smallGroups
			
			if len(d[dropID]) == 0:
				del d[dropID]

	elif dataName == "groupWorldDatas":
		for dropID, dropDatas in datas.items():
			d[dropID] = {}
			dropGroups = {}
			for name, value in dropDatas.items():
				if name == "no" or value == ():
					continue

				if name == "dropNum":
					d[dropID][name] = value
				else:
					dropGroups[name] = {"itemNo" : value[0],"itemNum" : value[1],"itemProb" : value[2],}

			d[dropID]["dropGroups"] = dropGroups

			if len(d[dropID]) == 0:
				del d[dropID]

	else:
		d = datas

	return d

# npc功能挂接表
NpcLinkFuncType = {
			1 : "npcShop",
			2 : "npcSkillStudy",
			3 : "npcLifeSkillStudy",
		} 
def funcNpcLink(mapDict, allDatas, datas, dataName):
	"""
	"""
	d = {}
	for no, linkInfo in datas.items():
		funcName = NpcLinkFuncType[linkInfo["funcType"]]
		if funcName == "npcShop":
			linkID = linkInfo["linkID"]
			if not d.get(linkID, False):
				d[linkID] = {}
				d[linkID]["items"] = {}
			itemID = None
			itemInfo = {}
			for name, value in linkInfo.items():
				if name == "no" or value == ():
					continue
				elif name == "itemID":
					itemID = value
				elif name in ("itemAmount", "itemRefreshTime", "itemBuyMin", "itemPriceType", "itemPrice", ): 
					if value > 0:
						itemInfo[name] = value
				else:
					d[linkID][name] = value

				if itemID:
					d[linkID]["items"][itemID] = itemInfo
		elif funcName  == "npcSkillStudy":
			linkID = linkInfo["linkID"]
			if not d.get(linkID, False):
				d[linkID] = {}
				d[linkID]["skills"] = ()
			for name, value in linkInfo.items():
				if name == "no" or value == ():
					continue
				elif name == "skillID":
					d[linkID]["skills"] += (value, )
				else:
					d[linkID][name] = value
		elif funcName == "npcLifeSkillStudy":
			linkID = linkInfo["linkID"]
			learnType = linkInfo["learnType"]
			if not d.get(linkID, False):
				d[linkID] = {}
				d[linkID]["skills"] = ()
				d[linkID]["formulas"] = ()
			for name, value in linkInfo.items():
				if name == "no" or name == "learnType" or value == ():
					continue
				elif name == "skillID":
					if learnType == 1:
						d[linkID]["skills"] += (value, )
					elif learnType == 2:
						d[linkID]["formulas"] += (value, )
				else:
					d[linkID][name] = value

	return d

# 装备强化表
def funcEquipReinfoce(mapDict, allDatas, datas, dataName):
	"""
	"""
	d = {}
	for no, eqInfo in datas.items():
		level = eqInfo["level"]
		if not d.get(level, False):
			d[level] = {}

		typeNO = eqInfo["typeNO"]
		if not d[level].get(typeNO, False):
			d[level][typeNO] = {}

		subTypeNO = eqInfo["subTypeNO"]
		if not d[level][typeNO].get(subTypeNO, False):
			d[level][typeNO][subTypeNO] = {}

		dd = d[level][typeNO][subTypeNO]
		for name, value in eqInfo.items():
			if name == "no" or name == "level" or \
				name == "typeNO" or name == "subTypeNO" or value == ():
				continue
			if name.find("stone") != -1:
				if not dd.get("stones", False):
					dd["stones"] = {}

				dd["stones"][value[0]] = value[1]
			else:
				dd[name] = value

	return d

# 装备强化分解表
def funcEquipDecomposeR(mapDict, allDatas, datas, dataName):
	"""
	"""
	d = {}
	for no, drInfo in datas.items():
		products = {}
		for productIdx in range(1, 6):
			productKey = "%s_%d" % ("product", productIdx)
			if drInfo[productKey] == ():
				continue
			products[productKey] = drInfo[productKey]

		if len(products) > 0:
			rLevel = drInfo["level"]
			d[rLevel] = {}
			d[rLevel]["products"] = products

	return d

# 装备普通分解表
def funcEquipDecomposeC(mapDict, allDatas, datas, dataName):
	"""
	"""
	d = {}
	for no, dcInfo in datas.items():
		eqTypeNo = dcInfo["eqTypeNO"]
		if not d.get(eqTypeNo, False):
			d[eqTypeNo] = {}

		eqSubTypeNo = dcInfo["eqSubTypeNO"]
		if not d[eqTypeNo].get(eqSubTypeNo, False):
			d[eqTypeNo][eqSubTypeNo] = {}

		dcTypeNo = dcInfo["dcTypeNO"]
		if not d[eqTypeNo][eqSubTypeNo].get(dcTypeNo, False):
			d[eqTypeNo][eqSubTypeNo][dcTypeNo] = {}

		dcTypeValue = dcInfo["dcTypeValue"]
		if not d[eqTypeNo][eqSubTypeNo][dcTypeNo].get(dcTypeValue, False):
			d[eqTypeNo][eqSubTypeNo][dcTypeNo][dcTypeValue] = {}

		dd = d[eqTypeNo][eqSubTypeNo][dcTypeNo][dcTypeValue]
		# 物品等级
		if dcTypeNo == 1:
			dcSubTypeNo = dcInfo["dcSubTypeNO"]
			dd[dcSubTypeNo] = {}
			dd = dd[dcSubTypeNo]
		# 物品ID
		elif dcTypeNo == 2:
			dd = dd

		products = {}
		for productIdx in range(1, 6):
			productKey = "%s_%d" % ("product", productIdx)
			if dcInfo[productKey] == ():
				continue
			products[productKey] = dcInfo[productKey]

		if len(products) > 0:
			dd["products"] = products

	return d

# 装备打造费用表
def funcEquipBuildCost(mapDict, allDatas, datas, dataName):
	"""
	"""
	d = {}
	if dataName == "costBaseDatas":
		for no, costInfo in datas.items():
			type = costInfo["type"]
			if not d.get(type, False):
				d[type] = {}

			level = costInfo["level"]
			d[type][level] = (costInfo["costType"], costInfo["cost"])

	elif dataName == "costFactorDatas":
		for no, costInfo in datas.items():
			type = costInfo["type"]
			if not d.get(type, False):
				d[type] = {}

			subType = costInfo["subType"]
			d[type][subType] = costInfo["factor"]

	return d

# 套装属性索引表
def funcSuitProp(mapDict, allDatas, datas, dataName):
	"""
	"""
	d = {}
	for suitID, suitDatas in datas.items():
		d[suitID] = {}
		d[suitID]["name"] = suitDatas["name"]
		d[suitID]["equips"] = suitDatas["equips"]
		dd = {}
		for propIdx in range(1, 11):
			eqCount = suitDatas["%s_%d" % ("activeProp", propIdx)]
			if eqCount == 0:
				break

			if not dd.get(eqCount, False):
				dd[eqCount] = []

			propID = suitDatas["%s_%d" % ("propID", propIdx)]
			propVal = suitDatas["%s_%d" % ("propVal", propIdx)]
			dd[eqCount].append((propID, propVal, ))

		d[suitID]["props"] = dd

	return d
