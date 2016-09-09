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

	html_template = "WebConsole/console_show_components.html"
	
	interfaces_groups = machinesmgr.queryAllInterfaces(request.session["sys_uid"], request.session["sys_user"])

	# [(machine, [components, ...]), ...]
	kbeComps = []
	for mID, comps in interfaces_groups.items():
		for comp in comps:
			if comp.componentType in VALID_CT:
				kbeComps.append( comp)

	context = {
		"KBEComps" : kbeComps,
	}
	return render( request, html_template, context )

@login_check
def connect( request ):
	"""
	控制台页面
	"""
	# 通过获取参数的方式校验参数是否存在
	GET = request.GET
	port = int( GET["port"] )
	ip = GET["ip"]
	title = GET["title"]

	ws_url = "ws://%s/wc/console/process_cmd?host=%s&port=%s" % ( request.META["HTTP_HOST"], ip, port )

	context = { "ws_url" : ws_url }
	return render( request, "WebConsole/console_connect.html", context )


from .telnet_console import TelnetConsole
from dwebsocket.decorators import accept_websocket

#@login_check
@accept_websocket
def process_cmd( request ):
	"""
	"""
	GET = request.GET
	port = int( GET["port"] )
	host = GET["host"]

	console = TelnetConsole(request.websocket, host, port)
	return console.run()

