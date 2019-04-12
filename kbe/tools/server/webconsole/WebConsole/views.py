# -*- coding: utf-8 -*-
import time, json, telnetlib, socket, sys, select
from django.shortcuts import render
from django.http import HttpResponse, HttpResponseBadRequest, HttpResponseRedirect
from django.core.exceptions import ObjectDoesNotExist
from django.conf import settings

from .models import ServerLayout
from pycommon import Machines, Define
from .machines_mgr import machinesmgr

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
	interfaces_groups = machinesmgr.queryAllInterfaces(request.session["sys_uid"], request.session["sys_user"])

	# [ [machine, other-components, ...], ...]
	kbeComps = []
	for mID, comps in interfaces_groups.items():
		if len( comps ) <= 1:
			continue

		dl = []
		kbeComps.append( dl )
		for comp in comps:
			d = {
				"ip"			: comp.intaddr,
				"componentType"	: comp.componentType,
				"componentName"	: comp.componentName,
				"fullname"		: comp.fullname,
				"uid"			: comp.uid,
				"pid"			: comp.pid,
				"componentID"	: comp.componentID,
				"globalOrderID"	: comp.globalOrderID,
				"cpu"			: comp.cpu,
				"mem"			: comp.mem,
				"usedmem"		: comp.usedmem,
				"entities"		: comp.entities,
				"proxies"		: comp.proxies,
				"clients"		: comp.clients,
				"consolePort"	: comp.consolePort,
			}
			dl.append( d )
	
	return HttpResponse( json.dumps( kbeComps ), content_type="application/json" )

def components_group_query( request , ct ):
	"""
	请求获取一组组件数据
	"""
	ct = int(ct)
	interfaces_groups = machinesmgr.queryAllInterfaces(request.session["sys_uid"], request.session["sys_user"])

	# [ [machine, other-components, ...], ...]
	kbeComps = []
	for mID, comps in interfaces_groups.items():
		if len( comps ) <= 1:
			continue

		dl = []
		kbeComps.append( dl )
		for comp in comps:
			if comp.componentType == ct or comp.componentType == 8:
				d = {
					"ip"			: comp.intaddr,
					"componentType"	: comp.componentType,
					"componentName"	: comp.componentName,
					"fullname"		: comp.fullname,
					"uid"			: comp.uid,
					"pid"			: comp.pid,
					"componentID"	: comp.componentID,
					"globalOrderID"	: comp.globalOrderID,
					"cpu"			: comp.cpu,
					"mem"			: comp.mem,
					"usedmem"		: comp.usedmem,
					"entities"		: comp.entities,
					"proxies"		: comp.proxies,
					"clients"		: comp.clients,
					"consolePort"	: comp.consolePort,
				}
				dl.append( d )
		
	return HttpResponse( json.dumps( kbeComps ), content_type="application/json" )

def components_one_query( request , ct, cid ):
	"""
	请求获取一个组件数据
	"""
	ct = int(ct)
	cid = int(cid)
	interfaces_groups = machinesmgr.queryAllInterfaces(request.session["sys_uid"], request.session["sys_user"])

	# [ [machine, other-components, ...], ...]
	kbeComps = []
	for mID, comps in interfaces_groups.items():
		if len( comps ) <= 1:
			continue

		dl = []
		kbeComps.append( dl )
		for comp in comps:
			if comp.componentType == 8:
				d = {
					"ip"			: comp.intaddr,
					"componentType"	: comp.componentType,
					"componentName"	: comp.componentName,
					"fullname"		: comp.fullname,
					"uid"			: comp.uid,
					"pid"			: comp.pid,
					"componentID"	: comp.componentID,
					"globalOrderID"	: comp.globalOrderID,
					"cpu"			: comp.cpu,
					"mem"			: comp.mem,
					"usedmem"		: comp.usedmem,
					"entities"		: comp.entities,
					"proxies"		: comp.proxies,
					"clients"		: comp.clients,
					"consolePort"	: comp.consolePort,
				}
				dl.append( d )

			if comp.componentID == cid:
				d = {
					"ip"			: comp.intaddr,
					"componentType"	: comp.componentType,
					"componentName"	: comp.componentName,
					"fullname"		: comp.fullname,
					"uid"			: comp.uid,
					"pid"			: comp.pid,
					"componentID"	: comp.componentID,
					"globalOrderID"	: comp.globalOrderID,
					"cpu"			: comp.cpu,
					"mem"			: comp.mem,
					"usedmem"		: comp.usedmem,
					"entities"		: comp.entities,
					"proxies"		: comp.proxies,
					"clients"		: comp.clients,
					"consolePort"	: comp.consolePort,
				}
				dl.append( d )
			
			if ct == 3:
				if comp.componentType == 6 or comp.componentType == 3:
					d = {
						"ip"			: comp.intaddr,
						"componentType"	: comp.componentType,
						"componentName"	: comp.componentName,
						"fullname"		: comp.fullname,
						"uid"			: comp.uid,
						"pid"			: comp.pid,
						"componentID"	: comp.componentID,
						"globalOrderID"	: comp.globalOrderID,
						"cpu"			: comp.cpu,
						"mem"			: comp.mem,
						"usedmem"		: comp.usedmem,
						"entities"		: comp.entities,
						"proxies"		: comp.proxies,
						"clients"		: comp.clients,
						"consolePort"	: comp.consolePort,
					}
					dl.append( d )
					
			if ct == 4:
				if comp.componentType == 5 or comp.componentType == 4:
					d = {
						"ip"			: comp.intaddr,
						"componentType"	: comp.componentType,
						"componentName"	: comp.componentName,
						"fullname"		: comp.fullname,
						"uid"			: comp.uid,
						"pid"			: comp.pid,
						"componentID"	: comp.componentID,
						"globalOrderID"	: comp.globalOrderID,
						"cpu"			: comp.cpu,
						"mem"			: comp.mem,
						"usedmem"		: comp.usedmem,
						"entities"		: comp.entities,
						"proxies"		: comp.proxies,
						"clients"		: comp.clients,
						"consolePort"	: comp.consolePort,
					}
					dl.append( d )
				
	return HttpResponse( json.dumps( kbeComps ), content_type="application/json" )

@login_check
def components_query_machines( request ):
	"""
	请求获取所有的machines
	"""
	machines = machinesmgr.queryMachines()

	# [ machine, ...]
	kbeComps = []
	for machine in components.machines:
		d = {
			"ip"	: machine.intaddr,
			"uid"	: machine.uid,
			"pid"	: machine.pid,
		}
		kbeComps.append( d )
	
	return HttpResponse( json.dumps( kbeComps ), content_type="application/json" )



@login_check
def components_manage( request ):
	"""
	组件管理主页面
	"""
	html_template = "WebConsole/components_manage.html"
	
	interfaces_groups = machinesmgr.queryAllInterfaces(request.session["sys_uid"], request.session["sys_user"])

	# [(machine, [components, ...]), ...]
	kbeComps = []
	for mID, comps in interfaces_groups.items():
		if len( comps ) > 1:
			kbeComps.extend( comps[1:] )

	context = {
		"KBEComps" : kbeComps,
		"hasComponents" : len( kbeComps ) > 0,
		"hasMachines" : len( interfaces_groups ) > 0,
	}
	return render( request, html_template, context )

@login_check
def components_run( request ):
	"""
	运行组件
	"""
	components = Machines.Machines( request.session["sys_uid"], request.session["sys_user"] )
	context = {}
	
	POST = request.POST
	if POST.get( "run", "" ):
		componentType = int( POST.get("componentType", "0") )
		targetMachine = POST.get("targetMachine", "").strip()
		runNumber = int( POST.get("runNumber", "0") )
		kbe_root = request.session["kbe_root"]
		kbe_res_path = request.session["kbe_res_path"]
		kbe_bin_path = request.session["kbe_bin_path"]
		
		if componentType not in Define.VALID_COMPONENT_TYPE_FOR_RUN or \
			not machinesmgr.hasMachine( targetMachine ) or \
			runNumber <= 0:
				context = { "error" : "invalid data!" }
		else:
			for e in range( runNumber ):
				cid = machinesmgr.makeCID( componentType )
				gus = machinesmgr.makeGUS( componentType )
				print("cid: %s, gus: %s" % (cid,gus))
				components.startServer( componentType, cid, gus, targetMachine, kbe_root, kbe_res_path, kbe_bin_path )

			time.sleep( 2 )
			return HttpResponseRedirect( "/wc/components/manage" )
	
	context["machines"] = machinesmgr.machines

	return render( request, "WebConsole/components_run.html", context )

@login_check
def components_stop( request, ct, cid ):
	"""
	停止一个组件
	"""
	ct = int(ct)
	cid = int(cid)
	
	components = Machines.Machines( request.session["sys_uid"], request.session["sys_user"] )

	components.stopServer( ct, componentID = cid, trycount = 0 )
	context = {
		"shutType": "stop_cid",
		"ct" : ct,
		"cid": cid
 	}
	return render( request, "WebConsole/components_shutdown.html", context )
	
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
	context = {
		"shutType": "all_ct"
 	}
	return render( request, "WebConsole/components_shutdown.html", context )

@login_check
def components_kill( request, ct, cid ):
	"""
	杀死一个组件进程
	"""
	ct = int(ct)
	cid = int(cid)
	
	components = Machines.Machines( request.session["sys_uid"], request.session["sys_user"] )

	components.killServer( ct, componentID = cid, trycount = 0 )
	context = {
		"shutType": "kill_cid",
		"ct" : ct,
		"cid": cid
 	}
	return render( request, "WebConsole/components_kill.html", context )

@login_check
def components_save_layout( request ):
	"""
	保存当前服务器运行状态
	"""
	layoutName = request.GET.get( "name" )
	if not layoutName:
		result = { "state" : "fault", "message" : "invalid layout name!!!" }
		return HttpResponse( json.dumps( result ), content_type="application/json" )
	
	VALID_CT = set( [
			Define.DBMGR_TYPE,
			Define.LOGINAPP_TYPE,
			Define.BASEAPPMGR_TYPE,
			Define.CELLAPPMGR_TYPE,
			Define.CELLAPP_TYPE,
			Define.BASEAPP_TYPE,
			Define.INTERFACES_TYPE,
			Define.LOGGER_TYPE,
		] )

	interfaces_groups = machinesmgr.queryAllInterfaces(request.session["sys_uid"], request.session["sys_user"])
	
	conf = {}
	
	for machineID, infos in interfaces_groups.items():
		for info in infos:
			if info.componentType not in VALID_CT:
				continue

			compnentName = Define.COMPONENT_NAME[info.componentType]
			if compnentName not in conf:
				conf[compnentName] = []
			d = { "ip" : info.intaddr, "cid" : info.componentID, "gus" : info.genuuid_sections }
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

@login_check
def components_show_layout( request ):
	"""
	显示保存的所有服务器运行配置
	"""
	VALID_CT = set( [
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
		for ct in VALID_CT:
			compnentName = Define.COMPONENT_NAME[ct]
			if compnentName not in layoutData:
				d[compnentName] = 0
			else:
				d[compnentName] = len( layoutData[compnentName] )
		datas.append( d )

	return render( request, "WebConsole/components_show_layout.html", { "KBELayouts" : datas } )

@login_check
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

@login_check
def components_load_layout( request ):
	"""
	加载某个保存的服务器运行配置，并启动服务器
	"""
	VALID_CT = set( [
			Define.DBMGR_TYPE,
			Define.LOGINAPP_TYPE,
			Define.BASEAPPMGR_TYPE,
			Define.CELLAPPMGR_TYPE,
			Define.CELLAPP_TYPE,
			Define.BASEAPP_TYPE,
			Define.INTERFACES_TYPE,
			Define.LOGGER_TYPE,
			
		] )
	kbe_root = request.session["kbe_root"]
	kbe_res_path = request.session["kbe_res_path"]
	kbe_bin_path = request.session["kbe_bin_path"]
	try:
		id = int( request.GET["id"] )
	except:
		id = 0
		
	if not id:
		return render( request, "WebConsole/components_load_layout.html", { "error" : "无效的参数" } )
	
	components = Machines.Machines( request.session["sys_uid"], request.session["sys_user"] )
	interfaces_groups = machinesmgr.queryAllInterfaces(request.session["sys_uid"], request.session["sys_user"])
	
	for mID, comps in interfaces_groups.items():
		if len( comps ) > 1:
			return render( request, "WebConsole/components_load_layout.html", { "error" : "服务器正在运行，不允许加载" } )

	# 计数器
	t2c				= [0,] * len(Define.COMPONENT_NAME)
	components_ct	= [0,] * len(Define.COMPONENT_NAME)
	components_cid	= [0,] * len(Define.COMPONENT_NAME)
	components_gus	= [0,] * len(Define.COMPONENT_NAME)
	ly = ServerLayout.objects.get(pk = id)
	layoutData = json.loads( ly.config )
	for ct in VALID_CT:
		compnentName = Define.COMPONENT_NAME[ct]
		components_ct[ct] = ct
		for comp in layoutData.get( compnentName, [] ):
			print("components_load_layout(), component data: %s" % comp)
			cid = comp["cid"]
			if cid <= 0:
				cid = machinesmgr.makeCID(ct)
			components_cid[ct] = cid
			
			gus = comp["gus"]
			if gus <= 0:
				gus = machinesmgr.makeGUS(ct)
			components_gus[ct] = gus
			t2c[ct] += 1
			components.startServer( ct, cid, gus, comp["ip"], kbe_root, kbe_res_path, kbe_bin_path, 0 )

	context = {
		"run_counter"		: str(t2c),
		"components_ct"		: json.dumps(components_ct),
		"components_cid"	: json.dumps(components_cid),
		"components_gus"	: json.dumps(components_gus),
		"components_ip"		: comp["ip"]
	}
	return render( request, "WebConsole/components_load_layout.html", context )

@login_check
def machines_show_all( request ):
	"""
	忽略用户，显示所有的machine
	"""
	interfaces_groups = machinesmgr.queryAllInterfaces(0, "WebConsole")

	targetIP = request.GET.get( "target", None )
	
	kbeComps = []
	for mID, comps in interfaces_groups.items():
		if len( comps ) > 1 and comps[0].intaddr == targetIP:
			kbeComps = comps[1:]
			break

	kbeMachines = machinesmgr.queryMachines()
	kbeMachines.sort(key = lambda info: info.intaddr)

	context = {
		"KBEMachines" : kbeMachines,
		"KBEComps" : kbeComps,
	}
	return render( request, "WebConsole/machines_show_all.html", context )

