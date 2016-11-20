# -*- coding: utf-8 -*-
import sys, os, getpass


# 服务器组件类型定义
UNKNOWN_COMPONENT_TYPE	= 0
DBMGR_TYPE				= 1
LOGINAPP_TYPE			= 2
BASEAPPMGR_TYPE			= 3
CELLAPPMGR_TYPE			= 4
CELLAPP_TYPE			= 5
BASEAPP_TYPE			= 6
CLIENT_TYPE				= 7
MACHINE_TYPE			= 8
CONSOLE_TYPE			= 9
LOGGER_TYPE				= 10
BOTS_TYPE				= 11
WATCHER_TYPE			= 12
INTERFACES_TYPE			= 13
COMPONENT_END_TYPE		= 14

VALID_COMPONENT_TYPE_FOR_RUN = [
	DBMGR_TYPE, 
	LOGINAPP_TYPE, 
	BASEAPPMGR_TYPE, 
	CELLAPPMGR_TYPE, 
	CELLAPP_TYPE, 
	BASEAPP_TYPE, 
	LOGGER_TYPE, 
	BOTS_TYPE, 
	INTERFACES_TYPE, 
]

# ComponentName to type
COMPONENT_NAME2TYPE = {
	"unknown"		: UNKNOWN_COMPONENT_TYPE,
	"dbmgr"			: DBMGR_TYPE,
	"loginapp"		: LOGINAPP_TYPE,
	"baseappmgr"	: BASEAPPMGR_TYPE,
	"cellappmgr"	: CELLAPPMGR_TYPE,
	"cellapp" 		: CELLAPP_TYPE,
	"baseapp" 		: BASEAPP_TYPE,
	"client" 		: CLIENT_TYPE,
	"machine"		: MACHINE_TYPE,
	"console" 		: CONSOLE_TYPE,
	"logger" 		: LOGGER_TYPE,
	"bots" 			: BOTS_TYPE,
	"watcher" 		: WATCHER_TYPE,
	"interfaces" 	: INTERFACES_TYPE,
}

# Component name
COMPONENT_NAME = (
	"unknown",
	"dbmgr",
	"loginapp",
	"baseappmgr",
	"cellappmgr",
	"cellapp",
	"baseapp",
	"client",
	"machine",
	"console",
	"logger",
	"bots",
	"watcher",
	"interfaces",
)

if sys.hexversion >= 0x02060000:
	import io
	def BytesIO(bytesData = "".encode("utf-8")):
		return io.BytesIO(bytesData)
else:
	import StringIO
	def BytesIO(bytesData = ""):
		return StringIO.StringIO(bytesData)

if sys.hexversion >= 0x03000000:
	import configparser
else:
	import ConfigParser as configparser


try:
	import pwd
except:
	pwd = lambda x : x
	
	class struct_passwd(object): pass
	
	pwd.struct_passwd = struct_passwd
	
	def _getpwnam(username):
		v = struct_passwd()
		v.pw_name = username
		v.pw_uid = -1
		return v
	
	def _getpwuid(uid):
		v = struct_passwd()
		v.pw_name = "unknown"
		v.pw_uid = uid
		return v
	
	pwd.getpwnam = _getpwnam
	pwd.getpwuid = _getpwuid

def getDefaultUID():
	"""
	"""
	try:
		if sys.platform == "win32":
			uid = int(os.environ.get('uid', -1))
		else:
			uid = os.getuid()
	except:
		uid = -1

	try:
		if uid < 0:
			pw = pwd.getpwnam(getpass.getuser())
			uid = pw.pw_uid
	except:
		pass
	
	if uid == -1:
		print("\n[ERROR]: UID is not set, The current is -1. Please refer to the http://kbengine.org/docs/installation.html " \
			"environment variable settings, about UID!\n Or manually specifying the UID parameter.\n")

	return uid
