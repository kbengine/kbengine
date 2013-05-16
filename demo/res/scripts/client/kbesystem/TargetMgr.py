# -*- coding: utf-8 -*-
import KBEngine
import KBExtra
import kbesystem
from KBEDebug import *

class TargetMgr:
	def __init__(self):
		self._currTargetID = 0
		self._preTargetID = 0
		
	def entity(self):
		"""
		获得entity
		"""
		return KBEngine.entities.get(self._currTargetID)
		
	def targetID(self):
		return self._currTargetID

	def preTargetID(self):
		return self._preTargetID
		
	def setTargetID(self, entityID):
		self._preTargetID = self._currTargetID
		self._currTargetID = entityID
		self.onTargetChanged()
		
	def onTargetChanged(self):
		kbesystem.eventMgr.fire("TargetMgr.onTargetChanged", self._currTargetID)