# -*- coding: utf-8 -*-
import KBEngine
import random
from KBEDebug import *
from SpaceCopy import SpaceCopy
import d_entities
import d_spaces
import random
import wtimer

_STATE_WAIT_INPUT = 0  # 等待输入战斗
_STATE_WAIT_NEXT = 1	# 等待下一回合。

class SpaceFightCopy(SpaceCopy):
	def __init__(self):
		SpaceCopy.__init__(self)
		
		self._state = _STATE_WAIT_INPUT
		
		# 添加一个timer5秒后战斗开始
		self.addTimer(5, 0, wtimer.TIMER_TYPE_FIGTH_READY)
		
		self.monsters = {}
		
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
			
	def startInputFigth(self):
		"""
		开始接受输入战斗
		"""
		self._state = _STATE_WAIT_INPUT
		self.addTimer(30, 0, wtimer.TIMER_TYPE_FIGTH_WATI_INPUT_TIMEOUT)
		
		for e in self.avatars.values():
			DEBUG_MSG("SpaceFightCopy::startInputFigth(%i-%i)" % (e.id, self.spaceID))
			if e.client:
				e.client.startInputFigth(30)
		
	def onFightBeginTimer(self, tid, tno):
		"""
		"""
		self.startInputFigth()

	def onFightInputTimeoutTimer(self, tid, tno):
		"""
		"""
		self.autoCommitFight()
		self._state = _STATE_WAIT_NEXT
	
	def autoCommitFight(self):
		"""
		自动提交战斗
		"""
		mon = list(self.monsters.keys())
		
		for e in self.avatars.values():
			self.commitFight(e.id, mon.id, 1)
			
	def commitFight(self, casterID, targetID, skillID):
		"""
		defined method.
		"""
		DEBUG_MSG("SpaceFightCopy::commitFight: targetID=%i, skillID=%i" % (targetID, skillID))
		
		if self._state != _STATE_WAIT_INPUT:
			DEBUG_MSG("SpaceFightCopy::commitFight: state not is _STATE_WAIT_INPUT.")
			return
			
		caster = self.avatars.get(casterID)
		mon = self.monsters.get(targetID)
		
		v = random.randint(0, 30)
		mon.addHP(-v)
		self.client.fightResult(casterID, targetID, 1, [{"eff":"addHP", "val" : -v}])
		
SpaceFightCopy._timermap = {}
SpaceFightCopy._timermap.update(SpaceCopy._timermap)
SpaceFightCopy._timermap[wtimer.TIMER_TYPE_FIGTH_READY] = SpaceFightCopy.onFightBeginTimer
SpaceFightCopy._timermap[wtimer.TIMER_TYPE_FIGTH_WATI_INPUT_TIMEOUT] = SpaceFightCopy.onFightInputTimeoutTimer
