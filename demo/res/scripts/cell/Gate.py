# -*- coding: utf-8 -*-
import random
import math
import time
import KBEngine
from KBEDebug import *
from GameObject import GameObject

class Gate(GameObject):
	def __init__(self):
		GameObject.__init__(self)
		self.name = "Gate%i" % self.id
			
	def onTimer(self, id, userArg):
		"""
		KBEngine method.
		使用addTimer后， 当时间到达则该接口被调用
		@param id		: addTimer 的返回值ID
		@param userArg	: addTimer 最后一个参数所给入的数据
		"""
		pass
	