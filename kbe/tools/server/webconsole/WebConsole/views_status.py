# -*- coding: utf-8 -*-
import time, json, sys
from django.shortcuts import render
from django.http import HttpResponse, HttpResponseBadRequest, HttpResponseRedirect
from django.core.exceptions import ObjectDoesNotExist
from django.conf import settings

from .models import ServerLayout
from pycommon import Define
from pycommon import Component_Status
from .machines_mgr import machinesmgr
from .auth import login_check

@login_check
def show_components( request ):
	"""
	控制台可连接的组件显示页面
	"""
	VALID_CT = set( [
		Define.BASEAPPMGR_TYPE, 
		Define.CELLAPPMGR_TYPE, 
		Define.DBMGR_TYPE,
		Define.LOGINAPP_TYPE,
		Define.CELLAPP_TYPE,
		Define.BASEAPP_TYPE,
		Define.INTERFACES_TYPE,
		Define.LOGGER_TYPE,
		] )

	html_template = "WebConsole/status_show_components.html"
	
	interfaces_groups = machinesmgr.queryAllInterfaces(request.session["sys_uid"], request.session["sys_user"])

	# [(machine, [components, ...]), ...]
	kbeComps = []
	for mID, comps in interfaces_groups.items():
		for comp in comps:
			if comp.componentType in VALID_CT:
				kbeComps.append( comp)

	context = {
		"http_host":request.META["HTTP_HOST"],
		"KBEComps" : kbeComps,
	}
	return render( request, html_template, context )


@login_check
def connect( request ):
	"""
	控制台可连接的组件显示页面
	"""
	VALID_CT = set( [
		Define.BASEAPPMGR_TYPE, 
		Define.CELLAPPMGR_TYPE, 
		Define.DBMGR_TYPE,
		Define.LOGINAPP_TYPE,
		Define.CELLAPP_TYPE,
		Define.BASEAPP_TYPE,
		Define.INTERFACES_TYPE,
		Define.LOGGER_TYPE,
	] )
	html_template = "WebConsole/status_connect.html"
	GET = request.GET
	try:
		cp_type = int(GET["cp"])
		cp_port = int(GET["port"])
		cp_host = GET["host"]
	except:
		context = {
			"err" : "进程未运行"
		}
		return render(request, html_template, context)

	if cp_type == 3:
		child_type = 6
	elif cp_type ==4:
		child_type = 5

	interfaces_groups = machinesmgr.queryAllInterfaces(request.session["sys_uid"], request.session["sys_user"])

	# [(machine, [components, ...]), ...]
	kbeComps = []
	for mID, comps in interfaces_groups.items():
		for comp in comps:
			if comp.componentType in VALID_CT:
				kbeComps.append( comp)

	ws_url = "ws://%s/wc/status/process_cmd?cp=%s&port=%s&host=%s" % ( request.META["HTTP_HOST"], cp_type, cp_port, cp_host )

	context = {
		"http_host": request.META["HTTP_HOST"],
		"KBEComps" : kbeComps,
		"ws_url"   : ws_url,
		"child_type" : child_type

	}
	return render( request, html_template, context )


from dwebsocket import accept_websocket

class CSData(object):
	def __init__(self, wInst, cp, host, port):
		self.wInst = wInst
		self.cp = cp
		self.port = port
		self.host = host
		self.Component_Status = Component_Status.ComponentStatus(cp)

	def do(self):
		self.Component_Status.connect(self.host,self.port)
		self.Component_Status.requireQueryCS()
		while True:
			# if self.Component_Status.CSData == []:
			self.Component_Status.processOne()
			time.sleep(0.5)
			self.wInst.send(str.encode(str(self.Component_Status.CSData)))
			# time.sleep(1)
			self.Component_Status.clearCSData()
			self.Component_Status.requireQueryCS()

	def close(self): 
		if self.wsInst: 
			self.wsInst.close()
		self.wsInst = None

@accept_websocket
def process_cmd( request ):
	"""
	"""
	GET = request.GET
	cp_type = int(GET["cp"])
	cp_port = int(GET["port"])
	cp_host = GET["host"]
	components_stastus = CSData(request.websocket, cp_type, cp_host, cp_port)
	components_stastus.do()
	return 
