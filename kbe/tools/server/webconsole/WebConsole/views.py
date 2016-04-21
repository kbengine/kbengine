# -*- coding: utf-8 -*-
import time, json
from django.shortcuts import render
from django.http import HttpResponse, HttpResponseBadRequest, HttpResponseRedirect
from django.core.exceptions import ObjectDoesNotExist

from .models import ServerLayout
from pycommon import Machines, Define

from .auth import login_check

@login_check
def index( request ):
	"""
	"""
	return HttpResponseRedirect( "/wc/components/manage" )

@login_check
def components_query( request ):
	"""
	请求获取组件数据
	"""
	components = Machines.Machines( request.session["sys_uid"], request.session["sys_user"] )
	components.queryAllInterfaces(timeout = 0.5)

	# [ [machine, other-components, ...], ...]
	kbeComps = []
	for mID, comps in components.interfaces_groups.items():
		if len( comps ) <= 1:
			continue

		dl = []
		kbeComps.append( dl )
		for comp in comps:
			d = {
				"ip"            : comp.intaddr,
				"componentType" : comp.componentType,
				"componentName" : comp.componentName,
				"fullname"      : comp.fullname,
				"uid"           : comp.uid,
				"pid"           : comp.pid,
				"componentID"   : comp.componentID,
				"globalOrderID" : comp.globalOrderID,
				"cpu"           : comp.cpu,
				"mem"           : comp.mem,
				"usedmem"       : comp.usedmem,
				"entities"      : comp.entities,
				"proxies"       : comp.proxies,
				"clients"       : comp.clients,
				"consolePort"   : comp.consolePort,
			}
			dl.append( d )
	
	return HttpResponse( json.dumps( kbeComps ), content_type="application/json" )

@login_check
def components_query_machines( request ):
	"""
	请求获取所有的machines
	"""
	components = Machines.Machines( 0, "WebConsole" )
	components.queryMachines()

	# [ machine, ...]
	kbeComps = []
	for machine in components.machines:
		d = {
			"ip"   : machine.intaddr,
			"uid"  : machine.uid,
			"pid"  : machine.pid,
		}
		kbeComps.append( d )
	
	return HttpResponse( json.dumps( kbeComps ), content_type="application/json" )



@login_check
def components_manage( request ):
	"""
	组件管理主页面
	"""
	html_template = "WebConsole/components_manage.html"
	
	components = Machines.Machines( request.session["sys_uid"], request.session["sys_user"] )
	components.queryAllInterfaces(timeout = 0.5)

	# [(machine, [components, ...]), ...]
	kbeComps = []
	for mID, comps in components.interfaces_groups.items():
		if len( comps ) > 1:
			kbeComps.append( ( comps[0], comps[1:]) )

	context = {
		"KBEComps" : kbeComps,
		"hasComponents" : len( kbeComps ) > 0,
		"hasMachines" : len( components.interfaces_groups ) > 0,
	}
	return render( request, html_template, context )

@login_check
def components_run( request ):
	"""
	运行组件
	"""
	components = Machines.Machines( request.session["sys_uid"], request.session["sys_user"] )
	components.queryAllInterfaces(timeout = 0.5)
	context = {}
	
	POST = request.POST
	if POST.get( "run", "" ):
		componentType = int( POST.get("componentType", "0") )
		targetMachine = POST.get("targetMachine", "").strip()
		runNumber = int( POST.get("runNumber", "0") )
		
		if componentType not in Define.VALID_COMPONENT_TYPE_FOR_RUN or \
			not components.hasMachine( targetMachine ) or \
			runNumber <= 0:
				context = { "error" : "invalid data!" }
		else:
			for e in range( runNumber ):
				cid = components.makeCID( componentType )
				gus = components.makeGUS( componentType )
				components.startServer( componentType, cid, gus, targetMachine )

			time.sleep( 2 )
			return HttpResponseRedirect( "/wc/components/manage" )
	
	context["machines"] = components.machines

	return render( request, "WebConsole/components_run.html", context )

@login_check
def components_stop( request ):
	"""
	停止一个组件
	"""
	components = Machines.Machines( request.session["sys_uid"], request.session["sys_user"] )
	components.queryAllInterfaces(timeout = 0.5)
	context = {}
	
	POST = request.POST
	
	# 当前Machine不支持停止单个组件

@login_check
def components_shutdown( request ):
	"""
	停止服务器
	"""
	COMPS_FOR_SHUTDOWN = [
		Define.BOTS_TYPE, 
		Define.LOGINAPP_TYPE, 
		Define.CELLAPP_TYPE, 
		Define.BASEAPP_TYPE, 
		Define.CELLAPPMGR_TYPE, 
		Define.BASEAPPMGR_TYPE, 
		Define.DBMGR_TYPE, 
		Define.INTERFACES_TYPE, 
		Define.LOGGER_TYPE, 
	]

	components = Machines.Machines( request.session["sys_uid"], request.session["sys_user"] )
	
	for ctid in COMPS_FOR_SHUTDOWN:
		components.stopServer( ctid, trycount = 0 )
	
	return render( request, "WebConsole/components_shutdown.html", {} )

def components_save_layout( request ):
	"""
	保存当前服务器运行状态
	"""
	layoutName = request.GET.get( "name" )
	if not layoutName:
		result = { "state" : "fault", "message" : "invalid layout name!!!" }
		return HttpResponse( json.dumps( result ), content_type="application/json" )
	
	VALIDATE_CT = set( [
			Define.DBMGR_TYPE,
			Define.LOGINAPP_TYPE,
			Define.BASEAPPMGR_TYPE,
			Define.CELLAPPMGR_TYPE,
			Define.CELLAPP_TYPE,
			Define.BASEAPP_TYPE,
			Define.INTERFACES_TYPE,
			Define.LOGGER_TYPE,
		] )

	components = Machines.Machines( request.session["sys_uid"], request.session["sys_user"] )
	components.queryAllInterfaces(timeout = 0.5)
	
	conf = {}
	
	for machineID, infos in components.interfaces_groups.items():
		for info in infos:
			if info.componentType not in VALIDATE_CT:
				continue

			compnentName = Define.COMPONENT_NAME[info.componentType]
			if compnentName not in conf:
				conf[compnentName] = []
			d = { "ip" : info.intaddr, "cid" : info.componentID, "gus" : 0 } # 当前取不到gus参数，所以只是先写0
			conf[compnentName].append( d )
	
	if len( conf ) == 0:
		result = { "state" : "fault", "message" : "当前没有服务器在运行!!!" }
		return HttpResponse( json.dumps( result ), content_type="application/json" )
	
	try:
		m = ServerLayout.objects.get(name = layoutName)
	except ObjectDoesNotExist:
		m = ServerLayout()
	
	m.name = layoutName
	m.sys_user = request.session["sys_user"]
	m.config = json.dumps( conf )
	m.save()
	
	result = { "state" : "success", "message" : "" }
	return HttpResponse( json.dumps( result ), content_type="application/json" )

def components_show_layout( request ):
	"""
	显示保存的所有服务器运行配置
	"""
	VALIDATE_CT = set( [
			Define.DBMGR_TYPE,
			Define.LOGINAPP_TYPE,
			Define.BASEAPPMGR_TYPE,
			Define.CELLAPPMGR_TYPE,
			Define.CELLAPP_TYPE,
			Define.BASEAPP_TYPE,
			Define.INTERFACES_TYPE,
			Define.LOGGER_TYPE,
		] )

	qs = ServerLayout.objects.all()
	datas = []
	for q in qs:
		d = {}
		d["id"] = q.id
		d["layout_name"] = q.name
		d["sys_user"] = q.sys_user
		layoutData = json.loads( q.config )
		for ct in VALIDATE_CT:
			compnentName = Define.COMPONENT_NAME[ct]
			if compnentName not in layoutData:
				d[compnentName] = 0
			else:
				d[compnentName] = len( layoutData[compnentName] )
		datas.append( d )

	return render( request, "WebConsole/components_show_layout.html", { "KBELayouts" : datas } )

def components_delete_layout( request ):
	"""
	删除某个保存的服务器运行配置
	"""
	try:
		id = int( request.GET["id"] )
	except:
		id = 0
		
	if not id:
		return HttpResponseRedirect( "/wc/components/show_layout" )

	ServerLayout.objects.filter(pk = id).delete();

	return HttpResponseRedirect( "/wc/components/show_layout" )

def components_load_layout( request ):
	"""
	加载某个保存的服务器运行配置，并启动服务器
	"""
	VALIDATE_CT = set( [
			Define.DBMGR_TYPE,
			Define.LOGINAPP_TYPE,
			Define.BASEAPPMGR_TYPE,
			Define.CELLAPPMGR_TYPE,
			Define.CELLAPP_TYPE,
			Define.BASEAPP_TYPE,
			Define.INTERFACES_TYPE,
			Define.LOGGER_TYPE,
		] )
	
	try:
		id = int( request.GET["id"] )
	except:
		id = 0
		
	if not id:
		return render( request, "WebConsole/components_load_layout.html", { "error" : "无效的参数" } )
	
	components = Machines.Machines( request.session["sys_uid"], request.session["sys_user"] )
	components.queryAllInterfaces(timeout = 0.5)
	
	for mID, comps in components.interfaces_groups.items():
		if len( comps ) > 1:
			return render( request, "WebConsole/components_load_layout.html", { "error" : "服务器正在运行，不允许加载" } )

	# 计数器
	t2c = [0,] * len(Define.COMPONENT_NAME)

	ly = ServerLayout.objects.get(pk = id)
	layoutData = json.loads( ly.config )
	for ct in VALIDATE_CT:
		compnentName = Define.COMPONENT_NAME[ct]
		for comp in layoutData.get( compnentName, [] ):
			cid = comp["cid"]
			if cid <= 0:
				cid = components.makeCID(ct)
			
			gus = comp["gus"]
			if gus <= 0:
				gus = components.makeGUS(ct)
			t2c[ct] += 1
			components.startServer( ct, cid, gus, comp["ip"], 0 )
	
	context = {
		"run_counter" : str(t2c)
	}
	return render( request, "WebConsole/components_load_layout.html", context )
