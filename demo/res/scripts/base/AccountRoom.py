# -*- coding:utf-8 -*-

from KBEDebug import *
import KBEngine

class AccountRoom( KBEngine.Base ):

	def __init__( self ):
		KBEngine.Base.__init__( self )
		self.accountDict = {}
		KBEngine.globalData["AccountRoom"] = self
		#self.registerGlobally( "AccountRoom", self.registerGlobalCB )
		INFO_MSG( "register globally success." )
		
	def registerGlobalCB( self, complete ):
		if not complete:	# 由于某些原因没注册成功（不知道何种原因，要研究一下），重新注册，直到成功或者尝试次数达到上限也无法注册
			self.registerGlobally( "AccountRoom", self.registerGlobalCB )
		else:
			INFO_MSG( "register globally success." )
			
	def registerAccount( self, accountName, accountMailbox ):
		if( self.accountDict is None):
			self.accountDict = {}
		#for name, mailbox in self.accountDict.iteritems():		# 一次性处理，这里可能需要分批处理，避免瓶颈。需要有测试数据支持。
		for name, mailbox in self.accountDict.items():
			mailbox.client.onNewAccountEnter( accountName )
			accountMailbox.client.onNewAccountEnter( name )	# 这个可以考虑一次性处理，一个方法内发送，不需要多次调用方法，利弊要分析一下
		self.accountDict[accountName] = accountMailbox
		INFO_MSG( "%s register to chatroom success" % accountName )
		
	def deregisterAccount( self, accountName ):
		"""
		"""
		del self.accountDict[accountName]
		for mailbox in self.accountDict.values():
			mailbox.client.onAccountLeave( accountName )
		INFO_MSG( "%s deregister from chat room" % accountName )
		
	def sendMsg( self, accountName, msg ):
		"""
		"""
		for name, mailbox in self.accountDict.items():
			if True or name != accountName:
				mailbox.client.receiveMsg( accountName, msg )
				