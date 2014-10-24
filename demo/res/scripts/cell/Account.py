# -*- coding: utf-8 -*-
import KBEngine
from KBEDebug import *
INFO_MSG(str.format('exec file: {}....', __file__))
class Account(KBEngine.Entity):
	def __init__(self):
		KBEngine.Entity.__init__(self)
