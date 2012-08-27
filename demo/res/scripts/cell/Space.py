# -*- coding: utf-8 -*-
import KBEngine
from KBEDebug import *

class Space(KBEngine.Entity):
	def __init__(self):
		KBEngine.Entity.__init__(self)
		DEBUG_MSG('创建了一个space[%d] entityID = %i.' % (self.spaceUType, self.id))
		#self.addSpaceGeometryMapping(self.spaceID, self.spaceName)