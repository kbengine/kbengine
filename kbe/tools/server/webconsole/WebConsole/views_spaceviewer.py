# -*- coding: utf-8 -*-
import time, sys
import traceback

try:
	import json
except:
	# 如果没有json模块，则尝试导入simplejson模块（例如py2.5）
	import simplejson as json

from django.shortcuts import render
from django.http import HttpResponse, HttpResponseBadRequest, HttpResponseRedirect
from django.core.exceptions import ObjectDoesNotExist
from django.conf import settings

from .models import ServerLayout
from pycommon import Define, SpaceViews
from .machines_mgr import machinesmgr
from .auth import login_check

@login_check
def show_components( request ):
	"""
	控制台可连接的组件显示页面
	"""
	VALID_CT = set( [
		Define.CELLAPPMGR_TYPE,
		Define.CELLAPP_TYPE
		] )

	html_template = "WebConsole/spaceviewer.html"
	
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
		componentType = kbeComps[0].componentType
		componentName = kbeComps[0].componentName
		uid = request.session["sys_uid"]
	except:
		context = {
			"err" : "cellappmgr进程未运行"
		}
		return render( request, html_template, context )
	ws_url = "ws://%s/wc/spaceviewer/process_cmd?host=%s&port=%s&cp=%s" % ( request.META["HTTP_HOST"], intaddr, intport, componentType)
	context = {
		"http_host":request.META["HTTP_HOST"],
		"kbeComps":kbeComps,
		"intaddr": intaddr,
		"intport": intport,
		"extaddr":extaddr,
		"extport":extport,
		"componentType":componentType,
		"componentName":componentName,
		"uid" : uid,
		"ws_url":ws_url
	}
	return render(request, html_template, context)

from dwebsocket import accept_websocket

class spaceviewerDate(object):
	def __init__(self, wInst, cp, host, port):
		self.wInst = wInst
		self.cp = cp
		self.port = port
		self.host = host

	def do(self):
		self.SpaceViews = SpaceViews.SpaceViewer(self.cp)
		self.SpaceViews.connect(self.host,self.port)
		self.test = ""
		while True:
			self.SpaceViews.requireQuerySpaceViewer()
			self.SpaceViews.processOne(0.1)
			if self.SpaceViews.SpaceViewerData != self.test:
				data = json.dumps(self.SpaceViews.SpaceViewerData)
				self.wInst.send(data.encode())
				self.test = self.SpaceViews.SpaceViewerData
			self.SpaceViews.clearSpaceViewerData()

	def close(self): 
		if self.wsInst: 
			self.wsInst.close()
		self.wsInst = None

class CellSpace(object):
	def __init__(self, wInst, cp, host, port, spaceID):
		self.wInst = wInst
		self.cp = cp
		self.port = port
		self.host = host
		self.spaceID = spaceID
		self.test = ""
	def do(self):
		self.CellSpaceViewer = SpaceViews.CellViewer(self.cp, self.spaceID)
		self.CellSpaceViewer.connect(self.host,self.port)
		while True:
			self.CellSpaceViewer.requireQueryCellViewer()
			self.CellSpaceViewer.processOne(0.1)
			if self.CellSpaceViewer.CellViewerData != self.test:
				data = json.dumps(self.CellSpaceViewer.CellViewerData)
				self.wInst.send(data.encode())
				self.test = self.CellSpaceViewer.CellViewerData
			self.CellSpaceViewer.clearCellViewerData()

	def close(self): 
		if self.wsInst: 
			self.wsInst.close()
		self.wsInst = None

@accept_websocket
def process_cmd( request ):
	"""
	"""
	try:
		GET = request.GET
		cp_host = GET["host"]
		cp_port = int(GET["port"])
		cp_type = int(GET["cp"])
		SpaceViewers = spaceviewerDate(request.websocket, cp_type, cp_host, cp_port)
		SpaceViewers.do()
	except:
		log = '\n'.join(traceback.format_exception(*(sys.exc_info())))
		print(log)
	return 

@accept_websocket
def cell_process_cmd( request ):
	"""
	"""
	try:
		GET = request.GET
		cp_host = GET["host"]
		cp_port = int(GET["port"])
		cp_type = int(GET["cp"])
		spaceID = int(GET["spaceID"])
		CellSpaceViewer = CellSpace(request.websocket, cp_type, cp_host, cp_port, spaceID)
		CellSpaceViewer.do()
	except:
		log = '\n'.join(traceback.format_exception(*(sys.exc_info())))
		print(log)
	return 
