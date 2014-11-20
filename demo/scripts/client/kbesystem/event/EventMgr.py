# -*- coding: utf-8 -*-
import KBEngine
import KBExtra
from KBEDebug import *

class EventMgr:
	def __init__(self):
		self._events = {}
		
	def registerEventHandler(self, eventID, eventhandler):
		"""
		获得entity
		"""
		if eventID not in self._events:
			self._events[eventID] = [eventhandler]
		else:
			self._events[eventID].append(eventhandler)
		
	def fire(self, eventID, *args):
		for eventhandler in self._events.get(eventID, []):
			eventhandler(*args)