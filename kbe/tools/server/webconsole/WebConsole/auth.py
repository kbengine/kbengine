# -*- coding: utf-8 -*-

import functools, hashlib
from django.shortcuts import render
from django.http import HttpResponse, HttpResponseBadRequest, HttpResponseRedirect
from django.core.exceptions import ObjectDoesNotExist
from django.contrib.auth.hashers import check_password, make_password
from django.conf import settings

from .models import AuthUser

def login_check(func):
	"""
	decorator.
	检查是否已登录
	"""
	@functools.wraps(func)
	def wrapper(request, *args, **kws):
		if not request.session.get( "logined", False ):
			return HttpResponseRedirect( "/wc/login" )
		
		# 管理者想进普通用户页面就必须先登出
		if request.session.get( "auth_is_admin" ):
			return HttpResponseRedirect( "/wc/logout" )
		return func( request, *args, **kws )
	
	return wrapper

def admin_check(func):
	"""
	decorator.
	检查是否已登录
	"""
	@functools.wraps(func)
	def wrapper(request, *args, **kws):
		if not request.session.get( "logined", False ):
			return HttpResponseRedirect( "/wc/login" )
		
		# 普通用户想进用户管理页面就必须先登出
		if not request.session.get( "auth_is_admin" ):
			return HttpResponseRedirect( "/wc/logout" )
		return func( request, *args, **kws )
	
	return wrapper


def login(request):
	"""
	账号登录页面
	"""
	if request.session.get( "logined", False ):
			return HttpResponseRedirect( "/wc/index" )

	name = request.POST.get("name", None)
	password = request.POST.get("password", None)
	context = { "next_uri" : "" }
	
	if name is None or password is None:
		return render(request, "WebConsole/login.html", context)
	
	if not name or not password:
		context["error"] = "错误：缺少用户名或密码参数"
		return render(request, "WebConsole/login.html", context)
	
	try:
		user = AuthUser.objects.get(name = name)
	except ObjectDoesNotExist:
		# 如果找不到特定的账号，并且账号密码都是默认值，则进入管理页面
		if name == "Admin" and password == "123456":
			pwd = make_password( "123456" )
			user = AuthUser( name = name, show_name = name, password = pwd, sys_user = "UNKNOWN", sys_uid = -1 )
			user.save()
		else:
			context["error"] = "错误：账号不存在"
			return render(request, "WebConsole/login.html", context)

	if not check_password( password, user.password ):
		context["error"] = "错误：密码不正确"
		return render(request, "WebConsole/login.html", context)

	request.session["logined"] = True
	request.session["auth_id"] = user.id
	request.session["auth_user"] = name
	request.session["auth_show_name"] = user.show_name
	request.session["sys_user"] = user.sys_user
	request.session["sys_uid"] = user.sys_uid
	request.session["auth_is_admin"] = (name == "Admin")
	
	if 	request.session["auth_is_admin"]:
		return HttpResponseRedirect( "/wc/user/manage" )
	else:
		request.session["kbe_root"] = user.kbe_root
		request.session["kbe_res_path"] = user.kbe_res_path
		request.session["kbe_bin_path"] = user.kbe_bin_path
		return HttpResponseRedirect( "/wc/index" )


def logout(request):
	"""
	账号登出页面
	"""
	request.session.clear()
	
	return HttpResponseRedirect( "/wc/index" )

@admin_check
def user_manage( request ):
	"""
	账号管理
	"""
	users = AuthUser.objects.all()
	
	context = { "Users" : users }
	return render(request, "User/manage_user.html", context)

@admin_check
def user_add( request ):
	"""
	増加新用户
	"""
	html_template = "User/new_user.html"
	POST = request.POST
	context = {}
	if POST.get( "commit", "" ):
		username = POST.get("username", "")
		showname = POST.get("showname", "")
		password1 = POST.get("password1", "")
		password2 = POST.get("password2", "")
		sysuser = POST.get("sysuser", "")
		sysuid = POST.get("sysuid", "")
		kbe_root = POST.get("kbe-root", "")
		kbe_res_path = POST.get("kbe-res-path", "")
		kbe_bin_path = POST.get("kbe-bin-path", "")
		
		if not username:
			context["error"] = "无效的账号名"
			return render(request, html_template, context)
			
		if not showname:
			context["error"] = "无效的显示名"
			return render(request, html_template, context)
			
		if not password1:
			context["error"] = "密码不能为空"
			return render(request, html_template, context)
			
		if password1 != password2:
			context["error"] = "密码不正确"
			return render(request, html_template, context)
			
		try:
			sysuid = int( POST.get("sysuid", "0") )
		except:
			context["error"] = "无效的uid"
			return render(request, html_template, context)
			
		if not sysuser:
			context["error"] = "系统用户名不能为空"
			return render(request, html_template, context)
			
		if sysuid < 0:
			context["error"] = "无效的uid"
			return render(request, html_template, context)
			
		if AuthUser.objects.filter(name = username).exists():
			context["error"] = "账号名“%s”已存在" % username
			return render(request, html_template, context)
		
		pwd = make_password( password1 )
		user = AuthUser( name = username, show_name = showname, password = pwd, sys_user = sysuser, sys_uid = sysuid, kbe_root = kbe_root, kbe_res_path = kbe_res_path, kbe_bin_path = kbe_bin_path )
		user.save()
		
		return HttpResponseRedirect( "/wc/user/manage" )
	else:
		return render(request, html_template, context)
	
@admin_check
def user_delete( request ):
	"""
	删除用户
	"""
	try:
		id = int( request.GET.get( "id", "0" ) )
	except:
		return HttpResponseRedirect( "/wc/user/manage" )
	
	AuthUser.objects.filter(pk = id).delete()
	return HttpResponseRedirect( "/wc/user/manage" )

@admin_check
def change_pwd( request ):
	"""
	修改账号密码
	"""
	html_template = "User/change_pwd.html"
	
	name = request.POST.get( "username", "" )
	id = request.POST.get( "id", "" )
	commit = request.POST.get( "commit", "" )
	
	if not id or not name:
		return HttpResponseRedirect( "/wc/user/manage" )
	
	context = {}
	if not commit:
		return render(request, html_template, context)
	
	password1 = request.POST.get( "password1", "" )
	password2 = request.POST.get( "password2", "" )
	if not password1:
		context["error"] = "密码不能为空"
		return render(request, html_template, context)
		
	if password1 != password2:
		context["error"] = "密码不正确"
		return render(request, html_template, context)
	
	if not AuthUser.objects.filter(pk = id).exists():
		context["error"] = "账号名“%s”不存在" % name
		return render(request, html_template, context)
	
	pwd = make_password( password1 )
	AuthUser.objects.filter(pk = id).update( password = pwd )
	context["error"] = "修改完成"
	return render(request, html_template, context)

@admin_check
def change_user(request,userID):
	html_template = "User/change_user.html"
	POST = request.POST
	commit = POST.get( "commit", "" )
	id = userID
	if id =="" or not id:
		context["error"] = "无效ID"
		return render(request, html_template, context)
	user = AuthUser.objects.get(pk = id)
	context = {}
	if commit == "" or not commit:
		showname     = user.show_name
		sysuser      = user.sys_user 
		sysuid       = user.sys_uid
		kbe_root     = user.kbe_root
		kbe_res_path = user.kbe_res_path
		kbe_bin_path = user.kbe_bin_path

		context = {
			"showname": showname,
			"sysuser" : sysuser,
			"sysuid"  : sysuid,
			"kbe_root": kbe_root,
			"kbe_res_path" : kbe_res_path,
			"kbe_bin_path" : kbe_bin_path
		}
		return render(request, html_template, context)

	showname = POST.get("showname", "")
	sysuser = POST.get("sysuser", "")
	sysuid = POST.get("sysuid", "")
	kbe_root = POST.get("kbe-root", "")
	kbe_res_path = POST.get("kbe-res-path", "")
	kbe_bin_path = POST.get("kbe-bin-path", "")
		
	if not showname:
		context["error"] = "无效的昵称"
		return render(request, html_template, context)

	try:
		sysuid = int( POST.get("sysuid", "0") )
	except:
		context["error"] = "无效的uid"
		return render(request, html_template, context)
		
	if not sysuser:
		context["error"] = "系统用户名不能为空"
		return render(request, html_template, context)
		
	if sysuid < 0:
		context["error"] = "无效的uid"
		return render(request, html_template, context)
	AuthUser.objects.filter(pk = id).update(show_name = showname, sys_user = sysuser, sys_uid = sysuid, kbe_root = kbe_root, kbe_res_path = kbe_res_path, kbe_bin_path = kbe_bin_path )
	context["error"] = "修改完成"
	return render(request, html_template, context)



