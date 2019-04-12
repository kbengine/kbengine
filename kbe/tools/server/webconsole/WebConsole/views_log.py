# -*- coding: utf-8 -*-
from django.shortcuts import render
from django.http import HttpResponse, HttpResponseBadRequest, HttpResponseRedirect
from django.core.exceptions import ObjectDoesNotExist
from django.conf import settings

from .models import ServerLayout
from pycommon import Define
from .machines_mgr import machinesmgr
from .auth import login_check

@login_check
def connect(request):
	VALID_CT = set( [ Define.LOGGER_TYPE,] )	

	html_template = "WebConsole/log_connect.html"

	interfaces_groups = machinesmgr.queryAllInterfaces(request.session["sys_uid"], request.session["sys_user"])

	# [(machine, [components, ...]), ...]
	kbeComps = []
	for mID, comps in interfaces_groups.items():
		for comp in comps:
			if comp.componentType in VALID_CT:
				kbeComps.append( comp)
	POST = request.POST
	try:
		intaddr = kbeComps[0].intaddr
		intport = kbeComps[0].intport
		extaddr = kbeComps[0].extaddr
		extport = kbeComps[0].extport
		# host = kbeComps[0].extaddr
		# port = kbeComps[0].consolePort
		uid = request.session["sys_uid"]
	except:
		context = {
			"unlogger" : "logger进程未运行"
		}
		return render(request, html_template, context)

	#获取进程选中状态
	components_checks = [0,0,0,0,0,0,0,0,0,0,0,0,0,0]
	components_checks2 = [0,0,0,0,0,0,0,0,0,0,0,0,0,0]
	baseapp_check = POST.get("baseapp_check")
	baseappmgr_check = POST.get("baseappmgr_check")
	cellapp_check = POST.get("cellapp_check")
	dbmgr_check = POST.get("dbmgr_check")
	loginapp_check  = POST.get("loginapp_check")
	pull_state = POST.get("pull_state")

	if pull_state == 0:
		pull_state = 1

	if baseapp_check: components_checks[6] = Define.BASEAPP_TYPE
	if baseappmgr_check: components_checks[3] = Define.BASEAPPMGR_TYPE
	if cellapp_check: components_checks[5] = Define.CELLAPP_TYPE
	if dbmgr_check: components_checks[1] = Define.DBMGR_TYPE
	if loginapp_check: components_checks[2] = Define.LOGINAPP_TYPE
	if components_checks[6] == 0 \
	and components_checks[3] == 0 \
	and components_checks[5] == 0 \
	and components_checks[1] == 0 \
	and components_checks[2] == 0 :
		components_checks[6] = Define.BASEAPP_TYPE
		components_checks[3] = Define.BASEAPPMGR_TYPE
		components_checks[5] = Define.CELLAPP_TYPE
		components_checks[1] = Define.DBMGR_TYPE
		components_checks[2] = Define.LOGINAPP_TYPE
		baseapp_check 		 = 1
		baseappmgr_check 	 = 1
		cellapp_check 		 = 1
		dbmgr_check 		 = 1
		loginapp_check 		 = 1

	# if len(components_checks)<=1:components_checks[].append(Define.COMPONENT_END_TYPE)

	#获取log类型
	CRITICAL_check = 0
	DEBUG_check = 0
	ERROR_check = 0
	INFO_check = 0
	PRINT_check  = 0
	S_DBG_check = 0
	S_ERR_check = 0
	S_INFO_check = 0
	S_NORM_check = 0
	S_WARN_check = 0
	WARNING_check = 0
	logtype = 0x00000000
	CRITICAL = POST.get("CRITICAL")
	DEBUG = POST.get("DEBUG")
	ERROR = POST.get("ERROR")
	INFO = POST.get("INFO")
	PRINT = POST.get("PRINT")
	S_DBG = POST.get("S_DBG")
	S_ERR = POST.get("S_ERR")
	S_INFO = POST.get("S_INFO")
	S_NORM = POST.get("S_NORM")
	S_WARN = POST.get("S_WARN")
	WARNING = POST.get("WARNING")

	if CRITICAL:
		logtype |= logName2type["CRITICAL"]
		CRITICAL_check = 1
	if DEBUG: 
		logtype |= logName2type["DEBUG"]
		DEBUG_check = 1
	if ERROR: 
		logtype |= logName2type["ERROR"]
		ERROR_check = 1
	if INFO: 
		logtype |= logName2type["INFO"]
		INFO_check = 1
	if PRINT: 
		logtype |= logName2type["PRINT"]
		PRINT_check = 1
	if S_DBG: 
		logtype |= logName2type["S_DBG"]
		S_DBG_check = 1
	if S_ERR: 
		logtype |= logName2type["S_ERR"]
		S_ERR_check = 1
	if S_INFO: 
		logtype |= logName2type["S_INFO"]
		S_INFO_check = 1
	if S_NORM: 
		logtype |= logName2type["S_NORM"]
		S_NORM_check = 1
	if S_WARN: 
		logtype |= logName2type["S_WARN"]
		S_WARN_check = 1
	if WARNING: 
		logtype |= logName2type["S_WARN"]
		WARNING_check = 1
	if logtype == 0x00000000:
		logtype = 0xffffffff
		CRITICAL_check = 1
		DEBUG_check = 1
		ERROR_check = 1
		INFO_check = 1
		PRINT_check  = 1
		S_DBG_check = 1
		S_ERR_check = 1
		S_INFO_check = 1
		S_NORM_check = 1
		S_WARN_check = 1
		WARNING_check = 1

	#自定义搜索
	globalOrder = POST.get("globalOrder")
	groupOrder = POST.get("groupOrder")
	searchDate = POST.get("searchDate")
	keystr = POST.get("keystr")
	if globalOrder == None: globalOrder = ""
	if groupOrder == None: groupOrder = ""
	if searchDate == None: searchDate = ""
	if keystr == None: keystr = ""

	ws_url = "ws://%s/wc/log/process_cmd?&logtype=%s&extaddr=%s&extport=%s&uid=%s&baseapp_check=%s&baseappmgr_check=%s&cellapp_check=%s&dbmgr_check=%s&loginapp_check=%s&globalOrder=%s&groupOrder=%s&searchDate=%s&keystr=%s" % ( request.META["HTTP_HOST"], logtype, extaddr, extport, uid,baseapp_check,baseappmgr_check,cellapp_check,dbmgr_check,loginapp_check,globalOrder,groupOrder,searchDate,keystr )

	context = { 
		"ws_url" : ws_url,
		"baseapp_check"		: components_checks[6],
		"baseappmgr_check"	:components_checks[3],
		"cellapp_check"		:components_checks[5],
		"dbmgr_check"		:components_checks[1],
		"loginapp_check"	:components_checks[2],

		"CRITICAL_check"	:CRITICAL_check,
		"DEBUG_check"		:DEBUG_check,
		"ERROR_check"		:ERROR_check,
		"INFO_check"		:INFO_check,
		"PRINT_check"		:PRINT_check,
		"S_DBG_check"		:S_DBG_check,
		"S_ERR_check"		:S_ERR_check,
		"S_INFO_check"		:S_ERR_check,
		"S_NORM_check"		:S_NORM_check,
		"S_WARN_check"		:S_WARN_check,
		"WARNING_check"		:WARNING_check,
		"globalOrder"		:globalOrder,
		"groupOrder"		:groupOrder,
		"searchDate"		:searchDate,
		"keystr"			:keystr,

		"components_checks":components_checks,

		"pull_state" : pull_state
	}
	return render(request, html_template, context)

@login_check
def pull_log(request):
	VALID_CT = set( [ Define.LOGGER_TYPE,] )	
	interfaces_groups = machinesmgr.queryAllInterfaces(request.session["sys_uid"], request.session["sys_user"])

	# [(machine, [components, ...]), ...]
	kbeComps = []
	for mID, comps in interfaces_groups.items():
		for comp in comps:
			if comp.componentType in VALID_CT:
				kbeComps.append( comp)
	POST = request.POST
	try:
		intaddr = kbeComps[0].intaddr
		intport = kbeComps[0].intport
		extaddr = kbeComps[0].extaddr
		extport = kbeComps[0].extport
		# host = kbeComps[0].extaddr
		# port = kbeComps[0].consolePort
		uid = request.session["sys_uid"]
	except:
		message = {
			"unlogger" : "logger进程未运行"
		}
		return HttpResponse(json.dumps(message))
		# return render(request, html_template, context)

	#获取进程选中状态
	components_checks = [0,0,0,0,0,0,0,0,0,0,0,0,0,0]
	components_checks2 = [0,0,0,0,0,0,0,0,0,0,0,0,0,0]
	baseapp_check = POST.get("baseapp_check")
	baseappmgr_check = POST.get("baseappmgr_check")
	cellapp_check = POST.get("cellapp_check")
	dbmgr_check = POST.get("dbmgr_check")
	loginapp_check  = POST.get("loginapp_check")
	pull_state = POST.get("pull_state")

	if pull_state == 0:
		pull_state = 1

	if baseapp_check: components_checks[6] = Define.BASEAPP_TYPE
	if baseappmgr_check: components_checks[3] = Define.BASEAPPMGR_TYPE
	if cellapp_check: components_checks[5] = Define.CELLAPP_TYPE
	if dbmgr_check: components_checks[1] = Define.DBMGR_TYPE
	if loginapp_check: components_checks[2] = Define.LOGINAPP_TYPE
	if components_checks[6] == 0 \
	and components_checks[3] == 0 \
	and components_checks[5] == 0 \
	and components_checks[1] == 0 \
	and components_checks[2] == 0 :
		components_checks[6] = Define.BASEAPP_TYPE
		components_checks[3] = Define.BASEAPPMGR_TYPE
		components_checks[5] = Define.CELLAPP_TYPE
		components_checks[1] = Define.DBMGR_TYPE
		components_checks[2] = Define.LOGINAPP_TYPE
		baseapp_check 		 = 1
		baseappmgr_check 	 = 1
		cellapp_check 		 = 1
		dbmgr_check 		 = 1
		loginapp_check 		 = 1

	# if len(components_checks)<=1:components_checks[].append(Define.COMPONENT_END_TYPE)

	#获取log类型
	CRITICAL_check = 0
	DEBUG_check = 0
	ERROR_check = 0
	INFO_check = 0
	PRINT_check  = 0
	S_DBG_check = 0
	S_ERR_check = 0
	S_INFO_check = 0
	S_NORM_check = 0
	S_WARN_check = 0
	WARNING_check = 0
	logtype = 0x00000000
	CRITICAL = POST.get("CRITICAL")
	DEBUG = POST.get("DEBUG")
	ERROR = POST.get("ERROR")
	INFO = POST.get("INFO")
	PRINT = POST.get("PRINT")
	S_DBG = POST.get("S_DBG")
	S_ERR = POST.get("S_ERR")
	S_INFO = POST.get("S_INFO")
	S_NORM = POST.get("S_NORM")
	S_WARN = POST.get("S_WARN")
	WARNING = POST.get("WARNING")

	if CRITICAL:
		logtype |= logName2type["CRITICAL"]
		CRITICAL_check = 1
	if DEBUG: 
		logtype |= logName2type["DEBUG"]
		DEBUG_check = 1
	if ERROR: 
		logtype |= logName2type["ERROR"]
		ERROR_check = 1
	if INFO: 
		logtype |= logName2type["INFO"]
		INFO_check = 1
	if PRINT: 
		logtype |= logName2type["PRINT"]
		PRINT_check = 1
	if S_DBG: 
		logtype |= logName2type["S_DBG"]
		S_DBG_check = 1
	if S_ERR: 
		logtype |= logName2type["S_ERR"]
		S_ERR_check = 1
	if S_INFO: 
		logtype |= logName2type["S_INFO"]
		S_INFO_check = 1
	if S_NORM: 
		logtype |= logName2type["S_NORM"]
		S_NORM_check = 1
	if S_WARN: 
		logtype |= logName2type["S_WARN"]
		S_WARN_check = 1
	if WARNING: 
		logtype |= logName2type["S_WARN"]
		WARNING_check = 1
	if logtype == 0x00000000:
		logtype = 0xffffffff
		CRITICAL_check = 1
		DEBUG_check = 1
		ERROR_check = 1
		INFO_check = 1
		PRINT_check  = 1
		S_DBG_check = 1
		S_ERR_check = 1
		S_INFO_check = 1
		S_NORM_check = 1
		S_WARN_check = 1
		WARNING_check = 1

	#自定义搜索
	globalOrder = POST.get("globalOrder")
	groupOrder = POST.get("groupOrder")
	searchDate = POST.get("searchDate")
	keystr = POST.get("keystr")
	if globalOrder == None: globalOrder = ""
	if groupOrder == None: groupOrder = ""
	if searchDate == None: searchDate = ""
	if keystr == None: keystr = ""
	message = { 
		"ws_url" : ws_url,
		"baseapp_check"		: components_checks[6],
		"baseappmgr_check"	:components_checks[3],
		"cellapp_check"		:components_checks[5],
		"dbmgr_check"		:components_checks[1],
		"loginapp_check"	:components_checks[2],

		"CRITICAL_check"	:CRITICAL_check,
		"DEBUG_check"		:DEBUG_check,
		"ERROR_check"		:ERROR_check,
		"INFO_check"		:INFO_check,
		"PRINT_check"		:PRINT_check,
		"S_DBG_check"		:S_DBG_check,
		"S_ERR_check"		:S_ERR_check,
		"S_INFO_check"		:S_ERR_check,
		"S_NORM_check"		:S_NORM_check,
		"S_WARN_check"		:S_WARN_check,
		"WARNING_check"		:WARNING_check,
		"globalOrder"		:globalOrder,
		"groupOrder"		:groupOrder,
		"searchDate"		:searchDate,
		"keystr"			:keystr,

		"components_checks":components_checks,
	}

	return HttpResponse( json.dumps( message ) ,content_type='application/json' )

from pycommon.LoggerWatcher import LoggerWatcher, logName2type
from dwebsocket.decorators import accept_websocket

class LogWatch(object):
	"""
	日志输出
	"""
	def __init__(self,wsInst, extaddr, extport, uid, components_check, logtype, globalOrder, groupOrder, searchDate, keystr):
		self.wsInst = wsInst
		self.extaddr = extaddr
		self.extport = extport
		self.uid = uid
		self.components_check = components_check
		self.logtype = logtype
		self.globalOrder = globalOrder
		self.groupOrder = groupOrder
		self.searchDate = searchDate
		self.keystr = keystr
		self.logger = LoggerWatcher()
		self.previous_log = []

	def do(self):
		"""
		"""
		self.logger.close()
		self.logger.connect( self.extaddr, self.extport)
		self.logger.registerToLoggerForWeb( self.uid,self.components_check, self.logtype, self.globalOrder, self.groupOrder,self.searchDate, self.keystr )
		def onReceivedLog(logs):
			new_logs = list(set(logs)^set(self.previous_log))
			for e in new_logs:
				self.wsInst.send(e)
			self.previous_log = logs
		self.logger.receiveLog(onReceivedLog, True)
		
	def close(self):
		"""
		"""
		self.logger.close()	
		self.logger.deregisterFromLogger()

		if self.wsInst:
			self.wsInst.close()
		self.wsInst = None
		
		self.extaddr = ""
		self.extport = 0

#@login_check
@accept_websocket
def process_cmd( request ):
	"""
	"""
	
	GET = request.GET
	extaddr = GET["extaddr"]
	extport = int(GET["extport"])
	uid = int(GET["uid"])

	#获取进程选中状态
	components_check = [0,0,0,0,0,0,0,0,0,0,0,0,0,0]
	components_check2 = [0,0,0,0,0,0,0,0,0,0,0,0,0,0]
	baseapp_check = GET["baseapp_check"]
	baseappmgr_check = GET["baseappmgr_check"]
	cellapp_check = GET["cellapp_check"]
	dbmgr_check = GET["dbmgr_check"]
	loginapp_check  = GET["loginapp_check"]

	if baseapp_check == '1': components_check[6] = Define.BASEAPP_TYPE
	if baseappmgr_check == '1': components_check[3] = Define.BASEAPPMGR_TYPE
	if cellapp_check == '1': components_check[5] = Define.CELLAPP_TYPE
	if dbmgr_check == '1': components_check[1] = Define.DBMGR_TYPE
	if loginapp_check == '1': components_check[2] = Define.LOGINAPP_TYPE
	if components_check == components_check2:
		i=-1
		for x in components_check:
			i=i+1
			components_check[x] = i

	logtype = int(GET["logtype"])

	#自定义搜索
	globalOrder = GET["globalOrder"]
	groupOrder = GET["groupOrder"]
	if globalOrder == "" or globalOrder == None:
		globalOrder = 0
	if groupOrder == "" or groupOrder == None:
		groupOrder = 0

	globalOrder = int(globalOrder)
	groupOrder = int(groupOrder)
	searchDate = GET["searchDate"]
	keystr = GET["keystr"]
	console = LogWatch(request.websocket, extaddr, extport, uid, components_check, logtype, globalOrder, groupOrder, searchDate, keystr)
	console.do()
	return 
