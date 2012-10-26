# -*- coding: utf-8 -*-
import KBEngine
import random
from KBEDebug import *
from SpaceCopy import SpaceCopy
import d_entities
import d_spaces
import random
import wtimer

class SpaceFightCopy(SpaceCopy):
	def __init__(self):
		SpaceCopy.__init__(self)
	
		# 添加一个timer5秒后战斗开始
		self.addTimer(5, 0, wtimer.TIMER_TYPE_FIGTH_READY)
		
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
		self.addTimer(30, 0, wtimer.TIMER_TYPE_FIGTH_WATI_INPUT_TIMEOUT)
		
		self.base.startInputFigth()
		
	def onFightBeginTimer(self, tid, tno):
		"""
		"""
		self.startInputFigth()

	def onFightInputTimeoutTimer(self, tid, tno):
		"""
		"""
		self.startInputFigth()
		
SpaceFightCopy._timermap = {}
SpaceFightCopy._timermap.update(SpaceCopy._timermap)
SpaceFightCopy._timermap[wtimer.TIMER_TYPE_FIGTH_READY] = SpaceFightCopy.onFightBeginTimer
SpaceFightCopy._timermap[wtimer.TIMER_TYPE_FIGTH_WATI_INPUT_TIMEOUT] = SpaceFightCopy.onFightInputTimeoutTimer
