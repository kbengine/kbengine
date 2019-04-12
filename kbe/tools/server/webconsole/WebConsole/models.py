# -*- coding: utf-8 -*-

from django.db import models

class AuthUser( models.Model ):
	"""
	用户权限表
	
CREATE TABLE `User` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(128) NOT NULL COMMENT '账号',
  `user_name` varchar(128) NOT NULL COMMENT '显示名',
  `password` varchar(128) NOT NULL COMMENT '密码',
  `sys_user` varchar(128) NOT NULL COMMENT '系统账号',
  `sys_uid` int(10) NOT NULL COMMENT '系统账号ID',
  PRIMARY KEY (`id`),
  UNIQUE KEY `name` (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8
	"""
	id						= models.AutoField(primary_key = True)
	name					= models.CharField(max_length = 128, default = "", help_text = "账号", db_index = True, unique = True)
	show_name				= models.CharField(max_length = 128, default = "", help_text = "显示名")
	password				= models.CharField(max_length = 128, default = "", help_text = "密码")
	sys_user				= models.CharField(max_length = 128, default = "", help_text = "系统账号")
	sys_uid					= models.IntegerField(default = 0, help_text = "系统账号ID")
	kbe_root				= models.CharField(max_length = 256, default = "", help_text = "kbe_root")
	kbe_res_path			= models.CharField(max_length = 256, default = "", help_text = "kbe_res_path")
	kbe_bin_path			= models.CharField(max_length = 256, default = "", help_text = "kbe_bin_path")

class ServerLayout( models.Model ):
	"""
	服务器运行配置表
	"""
	id			= models.AutoField(primary_key = True)
	name		= models.CharField(max_length = 128, default = "", help_text = "名称", db_index = True, unique = True)
	sys_user	= models.CharField(max_length = 128, default = "", help_text = "系统账号")
	config		= models.TextField(max_length = 32768, default = "", help_text = "配置(JSON)")
