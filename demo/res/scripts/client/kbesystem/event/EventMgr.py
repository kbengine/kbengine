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
		if eventID in self._events:
			self._events[eventID] = [eventhandler]
		else:
			self._events[eventID].append(eventhandler)
		
	def fire(self, event, *args):
		for eventhandler in self._events.get(eventID, []):
			eventhandler.onEvent(event, *args)