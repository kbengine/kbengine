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

		self.monsters = {}
		self.inputAvatars = [] # 本回合已经提交过的人
		self.startRecvInputFigth()
		
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
				"dialogID" : mondatas.get("dialogID", 0),
				"name" : mondatas["name"],
				"descr" : mondatas.get("descr", ''),
			}
			
			e = KBEngine.createEntity(mondatas["entityType"], self.spaceID, (0,0,0), (0,0,0), params)
			self.monsters[e.id] = e
			
	def startRecvInputFigth(self):
		"""
		开始接受输入战斗
		"""
		self.inputAvatars = []
		self.addTimer(60, 0, wtimer.TIMER_TYPE_FIGTH_WATI_INPUT_TIMEOUT)

	def onFightInputTimeoutTimer(self, tid, tno):
		"""
		"""
		self.autoCommitFight()
		self.startRecvInputFigth()
		
	def autoCommitFight(self):
		"""
		自动提交战斗
		"""
		monID = random.choice(list(self.monsters.keys()))
		
		for e in self.avatars.values():
			if e.id not in self.inputAvatars:
				self.commitFight(e.id, monID, 1)
			
	def commitFight(self, casterID, targetID, skillID):
		"""
		defined method.
		"""
		DEBUG_MSG("SpaceFightCopy::commitFight: targetID=%i, skillID=%i" % (targetID, skillID))
		
		if casterID in self.inputAvatars:
			DEBUG_MSG("SpaceFightCopy::commitFight: casterID has inputAvatars.")
			return
		
		self.inputAvatars.append(casterID)
		
		caster = KBEngine.entities[casterID]
		mon = self.monsters.get(targetID)
		
		v = random.randint(0, 30)
		mon.addHP(-v)
		vv = random.randint(0, 30)
		caster.addHP(-vv)
		caster.client.fightResult(casterID, targetID, 1, [{"eff":"addHP", "val" : -v}])
		caster.client.fightResult(targetID, casterID, 1, [{"eff":"addHP", "val" : -vv}])
		caster.client.fightResult(0, 0, 0, [])

	def onEnter(self, entityMailbox):
		"""
		defined method.
		进入场景
		"""
		SpaceCopy.onEnter(self, entityMailbox)
		KBEngine.entities[entityMailbox.id].fightSeat = 2
		
SpaceFightCopy._timermap = {}
SpaceFightCopy._timermap.update(SpaceCopy._timermap)
SpaceFightCopy._timermap[wtimer.TIMER_TYPE_FIGTH_WATI_INPUT_TIMEOUT] = SpaceFightCopy.onFightInputTimeoutTimer
