# -*- coding: utf-8 -*-
import time, json, sys
from django.shortcuts import render
from django.http import HttpResponse, HttpResponseBadRequest, HttpResponseRedirect
from django.core.exceptions import ObjectDoesNotExist
from django.conf import settings

from .models import ServerLayout
from pycommon import Define
from pycommon import Watcher
from .machines_mgr import machinesmgr
from .auth import login_check

@login_check
def show_components( request ):
	"""
	控制台可连接的组件显示页面
	"""
	VALID_CT = set( [
			Define.DBMGR_TYPE,
			Define.LOGINAPP_TYPE,
			Define.CELLAPP_TYPE,
			Define.BASEAPP_TYPE,
			Define.INTERFACES_TYPE,
			Define.LOGGER_TYPE,
		] )

	html_template = "WebConsole/watcher_show_components.html"
	
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
	控制台页面
	"""
	html_template = "WebConsole/watcher_connect.html"
	GET = request.GET
	try:
		cp_type = int(GET["cp"])
		cp_port = int(GET["port"])
		cp_host = GET["host"]
		cp_key  = GET["key"]
	except:
		context = {
			"err" : "进程未运行"
		}
		return render(request, html_template, context)

	ws_url = "ws://%s/wc/watcher/process_cmd?cp=%s&port=%s&host=%s&key=%s" % ( request.META["HTTP_HOST"], cp_type, cp_port, cp_host, cp_key )

	context = { 
		"http_host":request.META["HTTP_HOST"],
		"ws_url" : ws_url,
	}
	return render( request, html_template, context )



from dwebsocket import accept_websocket

class WatcherData(object):
	def __init__(self, wInst, cp, port, host, key):
		self.wInst = wInst
		self.cp = cp
		self.port = port
		self.host = host
		self.key = key
		self.watcher = Watcher.Watcher(cp)

	def do(self):
		self.watcher.connect(self.host,self.port)
		self.watcher.requireQueryWatcher(self.key)
		while True:
			if self.watcher.watchData == []:
				self.watcher.processOne()
				if self.key == "root/network/messages":
					time.sleep(1)
				else:
					time.sleep(0.5)
			else:
				self.wInst.send(str.encode(str(self.watcher.watchData)))
				self.watcher.clearWatchData()
				self.watcher.requireQueryWatcher(self.key)

	def close(self):
		if self.wsInst:
			self.wsInst.close()
		self.wsInst = None

#@login_check
@accept_websocket
def process_cmd( request ):
	"""
	"""
	GET = request.GET
	cp_type = int(GET["cp"])
	cp_port = int(GET["port"])
	cp_host = GET["host"]
	cp_key  = GET["key"]
	watcher = WatcherData(request.websocket, cp_type, cp_port, cp_host, cp_key)
	watcher.do()
	return 