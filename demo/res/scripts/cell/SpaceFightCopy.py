# -*- coding: utf-8 -*-
import KBEngine
import random
from KBEDebug import *
from Space import Space
import d_entities
import d_spaces
import random

class SpaceFightCopy(Space):
	def __init__(self):
		Space.__init__(self)
	
		# 添加一个timer5秒后战斗开始
		self.addTimer(5, 0, 1)
		
		datas = d_spaces.datas[self.spaceUType]
		entitiesMinCount = datas.get("entitiesMinCount", 1)
		entitiesMaxCount = datas.get("entitiesMaxCount", 1)
		
		bosslist = []
		monlist = []
		
		for x in range(6):
			bossID = datas.get("boss%iID" % (x + 1), 0)
			bossLvMin = datas.get("boss%iLvMin" % (x + 1), 0)
			bossLvMax = datas.get("boss%iLvMax" % (x + 1), 0)
			if bossID > 0:
				bosslist.append((bossID, random.randint(bossLvMin, bossLvMax)))
			
			monID = datas.get("monster%iID" % (x + 1), 0)
			monLvMin = datas.get("monster%iLvMin" % (x + 1), 0)
			monLvMax = datas.get("monster%iLvMax" % (x + 1), 0)
			if monID > 0:
				monlist.append((monID, random.randint(monLvMin, monLvMax)))
			
		
		vals = bosslist + monlist
		random.shuffle(vals)
		
		for x in range(random.randint(entitiesMinCount, entitiesMaxCount)):
			val = vals[x]
			mondatas = d_entities.datas.get(val[0])

			params = {
				"fightSeat" : x,
				"uid" : mondatas["id"],
				"utype" : mondatas["etype"],
				"modelID" : mondatas["modelID"],
				"dialogID" : mondatas.get("dialogID"),
				"name" : mondatas["name"],
				"descr" : mondatas.get("descr", ''),
			}
			
			e = KBEngine.createEntity(mondatas["entityType"], self.spaceID, (0,0,0), (0,0,0), params)
	
	def startInputFigth(self):
		"""
		开始接受输入战斗
		"""
		self.addTimer(30, 0, 2)
		
		self.base.startInputFigth()
		
	def onTimer(self, id, userArg):
		"""
		KBEngine method.
		使用addTimer后， 当时间到达则该接口被调用
		@param id		: addTimer 的返回值ID
		@param userArg	: addTimer 最后一个参数所给入的数据
		"""
		if userArg == 1:
			self.startInputFigth()
		elif userArg == 2:
			pass