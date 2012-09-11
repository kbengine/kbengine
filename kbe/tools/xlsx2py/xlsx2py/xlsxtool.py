#-*- coding: gb2312 -*-
"""
工具
"""
import os
import sys
import xlsxError
from config import *
from xml.dom.minidom import *

def exportMenu(msgIndex, YCallback = None, NCallback = None, OCallback = None):
	"""
	简单的询问Y(yes),N(no)， Y:pass，N：exit, Other:continue
	now assume 输入 Y
	"""
	if YCallback:
		YCallback()
	return

	# xlsxError.info_input(msgIndex)
	
	# msg = "------------请输入Y(yes), N(no)"
	# if OCallback:
		# msg += ',other(o)'
	
	# print( msg,'------------------' )

	# print( "您的输入:" )

	# input_command = raw_input()

	# while(input_command):
		# if input_command.lower() == "y" or input_command == '\r':
			# if YCallback:YCallback()
			# return 
		# elif input_command.lower() == "n":
			# if NCallback:NCallback()
			# sys.exit(1)
		
		# elif input_command.lower() == "o":
			# if OCallback:OCallback()
			# return
		# else:
			# print( "是Y还是N：", )
			# input_command = raw_input()


def checkExtName(filePath, extName):
	"""
	检测扩展名，请将.也丢进来
	"""
	if filePath == "" or extName == '':
		return

	fileName = os.path.split(filePath)[-1]
	
	return fileName.rfind(extName) >= 0
		

def __checkPath(dirPath):
		"""
		必须有driverName E:,D: ==
		"""
		driveName = os.path.splitdrive(dirPath)[0]
		if not os.path.isdir(driveName):
			raise xlsxError.xe(EXPORT_ERROR_CPATH, (dirPath, ))

		__onCheckPath(dirPath)

		return
		
def __onCheckPath(dirPath):
	"""
	递归创建目录
	"""
	if not os.path.isdir(dirPath):
		prvdir = os.path.split(dirPath)[0]
		if not  os.path.isdir(prvdir):
			__onCheckPath(prvdir)
			
		os.mkdir(dirPath)

def createDir(dirPath):
	"""
	创建目录
	"""
	__checkPath(dirPath)	
	#__checkOkCreate(dirPath)

	return

def getFileMTime(fileName):
	return os.stat(fileName)[-2]

###########字符串处理####################
def inputList(var_list):
	"""
	"""
	for element in var_list:
		if isinstance(element, list):
			inputList(element)
		elif isinstance(element, str):
			inputElement(element)

def inputElement(element):
	"""
	对字串编码处理
	"""
	if isinstance(element, str):
		#element.strip().replace
		print( element )#.encode(FILE_CODE),
	#else:
		#print( element),

	return

def str2List(error_str, pywinerr_list):
	"""
	字符串 -> list,不检测括号的匹配
	"""
	begin_pos = error_str.find('(')
	next_pos = error_str.find(')')
	mid_pos = begin_pos

	if begin_pos > -1 and next_pos > -1:
		pywinerr_list.append([])
		suberr_list = pywinerr_list[-1]

		while next_pos > -1:
			mid_pos = error_str.find('(', mid_pos+1, next_pos)
			if mid_pos > -1:
				next_pos = error_str.find(')', next_pos+1)
			else:
				break

		str2List(error_str[begin_pos+1:next_pos], suberr_list)
		str2List(error_str[:begin_pos-1] + error_str[next_pos+1:], suberr_list)

	else:
		for strVal in error_str.split(","):
			pywinerr_list.append(strVal)

def val2Str(data):
	if isinstance(data, float):
		return str(int(data))
	if isinstance(data, bytes):
		return data.decode("utf-8")
	else:
		return data

################################################			
def list_to_text(ls):
	return tuple_to_text(ls)
	
def tuple_to_text(t):
	text = '('
	for x in t:		
		v = value_to_text(x)
		text += v + ', '
		
	text += ')'
	
	return text

depth = 1
def dict_to_text(d):
	text = '{'
	for k, v in d.iteritems():
		global depth
		depth += 1
		k = value_to_text(k)
		v = value_to_text(v)
		if v == 'None':
			continue
		if depth == 2:
			text += '\t' + k + ':' + v + ',\n'
		else:
			text += k+ ':' +v + ","
		depth -= 1

	text += '}'	
	return text

def value_to_text(v):
	if isinstance(v, str):
		return "'" + v.replace('\'', '\\\'') + "'"
	if isinstance(v, bytes):
		return v.decode("utf-8")
	if isinstance(v, dict):
		return dict_to_text(v)
		
	if isinstance(v, list):
		return list_to_text(v)
		
	if isinstance(v, tuple):
		return tuple_to_text(v)			
	
	return str(v)

#######################code############################
def toGBK(val):
	if isinstance(val, str):
		return val.encode("utf-8")
	return val
	
def GTOUC(val):
	return val
	
def STOU(val):
	"""
	SYS_CODE -> utf-8
	"""
	return val

def UTOF(val):
	"""
	utf-8 -> FILE_CODE
	"""
	return val

def FTOU(val):
	"""
	FILE_CODE ->UTF-8
	"""
	return val


