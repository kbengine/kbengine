# -*- coding: utf-8 -*-

"""
init extra path
"""
import os, sys

def initExtraRootPath():
	"""
	初始化扩展的目录，以加载其它的脚本
	"""
	appdir = os.path.dirname( os.path.abspath( __file__ ) )
	parentDir = os.path.dirname( appdir )
	parentDir = os.path.dirname( parentDir )
	if parentDir not in sys.path:
		sys.path.append( parentDir )
