# -*- coding: gb2312 -*-
#
import xlsxtool
def funcPos2D(mapDict, dctData, chilidDict, data):
	"""
	返回int数据
	"""
	if data is None or (type(data) == str and len(data) == 0):
		return ()

	data = str(data)

	return (int(data.split(",")[0]), 0, int(data.split(",")[1]))

def funcInt(mapDict, dctData, chilidDict, data):
	"""
	返回int数据
	"""
	try:
		v = eval(data)
	except:
		return 0

	if type(v) != int:
		return 0

	return v

def funcFloat(mapDict, dctData, chilidDict, data):
	"""
	返回float数据
	"""
	if data is None or (type(data) == str and len(data) == 0):
		return 0.0

	return float(data)

def funcStr(mapDict, dctData, chilidDict, data):
	"""
	返回字符串数据
	"""
	if data is None:
		return ""

	if type(data) == str:
		return data
	else:
		data = str(data)
		data = data.encode('utf8')
		return str(data)

def funcEval(mapDict, dctData, chilidDict, data):
	"""
	返回eval数据
	"""
	if data is None or (type(data) == str and len(data) == 0):
		return ""
	return eval(data)

def funcTupleInt(mapDict, dctData, chilidDict, data):
	"""
	返回tuple数据
	"""
	if data is None or (type(data) == str and len(data) == 0):
		return ()

	data = str(data)

	return tuple([int(e) for e in data.split(",") if len(e) > 0])

def funcTupleFloat(mapDict, dctData, chilidDict, data):
	"""
	返回tuple数据
	"""
	if data is None or (type(data) == str and len(data) == 0):
		return ()

	data = str(data)

	return tuple([float(e) for e in data.split(",") if len(e) > 0])
	
def funcDict(mapDict, dctData, chilidDict, data):
	"""
	返回dict数据
	"xx:1'2'3;fff:2'3'4"
	"""
	if data is None or (type(data) == str and len(data) == 0):
		return ''
	
	data = str(data)
	dict1 = {}
	for item in data.split(';'):
		if item != '':
			 e = item.split(':')
			 if len(e) == 1:
			 	 dict1[int(e[0])] = ()
			 elif len(e) == 2:
			 	 dict1[int(e[0])] = tuple([index for index in e[1].split('`') if index != ''])

	return dict1

def funcTupleStr(mapDict, dctData, chilidDict, data):
	"""
	返回tuple数据
	"""
	if data is None or (type(data) == str and len(data) == 0):
		return ()

	data = str(data)
	return tuple([e for e in data.split(",") if len(e) > 0])

def funcTupleEval(mapDict, dctData, chilidDict, data):
	"""
	返回tuple数据
	"""
	if data is None or (type(data) == str and len(data) == 0):
		return ()

	data = str(data)
	return tuple([eval(e) for e in data.split(",") if len(e) > 0])

def funcTupleEvalMD(mapDict, dctData, chilidDict, data):
	"""
	返回tuple数据 使用代对表
	"""
	if data is None or (type(data) == str and len(data) == 0):
		return ()
	
	data = str(data)
	try:
		return tuple([eval(mapDict[e.decode("gb2312")]) for e in data.split(",") if len(e) > 0])
	except Exception as errstr:
		print( "函数中发生错误:%s" % errstr)
		return ()
	
def funcTupleEval1(mapDict, dctData, chilidDict, data):
	"""
	返回tuple数据
	1'100/2'100/3'54
	"""
	if data is None or (type(data) == str and len(data) == 0):
		return ()

	data = str(data)
	ret = []
	for e in data.split("/"):
		try:
			i, v = e.split("'")
		except Exception as errstr:
			print( "函数中发生错误:%s" % errstr)
			continue
		ret.append((eval(i), eval(v)))
	return tuple(ret)
	
def funcBool(mapDict, dctData, chilidDict, data):
	"""
	返回布尔值
	"""
	if data is None or (type(data) == str and len(data) == 0):
		return False
	return int(data) > 0

def funcNotBool(mapDict, dctData, chilidDict, data):
	"""
	返回取反的布尔值
	"""
	return not funcBool(mapDict, dctData, chilidDict, data)

def funcNull(mapDict, dctData, chilidDict, data):
	"""
	什么也不做 直接返回
	"""
	return data

def funcZipFloat(mapDict, dctData, chilidDict, data):
	"""
	返回float数据
	"""
	if data is None or (type(data) == str and len(data) == 0):
		return 0

	return int(float(data) * 10000)

def funcUNZipFloat(mapDict, dctData, chilidDict, data):
	"""
	返回float数据
	"""
	if data is None or (type(data) == str and len(data) == 0):
		return 0.0

	return int(data) / 10000.0
	
def funcFlags(mapDict, dctData, chilidDict, data):
	"""
	返回标记组合数据
	比如： 想在excel上配置标记组合
	近程攻击:0x00000001
	远程攻击:0x00000002
	暴击:0x00000004
	用此函数可以输出多个标记组成一个uint32的数字
	"""
	val = 0
	for x in data.split(","):
		if len(x) > 0:
			val |= int(mapDict[x])

	return val
