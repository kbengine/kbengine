import KBEngine
from KBEDebug import *
INFO_MSG(str.format('exec file: {}....', __file__))
def getListOfRooms():
	rooms = []
	for entity in KBEngine.entities.values():
		if entity.className == "ChatRoom":
			rooms += [entity.name]
	return rooms

def getRoom(whichRoom):
	for entity in KBEngine.entities.values():
		if entity.className == "ChatRoom" and entity.name == whichRoom:
			return entity
	return None

class ChatRoom( KBEngine.Base ):
	def __init__( self ):
		KBEngine.Base.__init__( self )
		self.name = "chatRoom"
		self.members = {}
		KBEngine.globalData["ChatRoomMgr"] = self

	def broadcast( self, name, message ):
		for entity in self.members.values():
			entity.client.recvMsg( name, message )

	def enter( self, entity ):
		self.members[entity.id] = entity
		self.broadcast( 'system', "%s: %s has entered" % (self.name, entity.name) )

	def leave( self, entityId ):
		del self.members[entityId]
		self.broadcast( 'system', "%s: %d is leaving" % (self.name, entityId) )

# ChatRoom.py
