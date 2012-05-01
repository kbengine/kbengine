# -*- coding: utf-8 -*-
import sys

def getClassName(frame):
	try:
		# 取得目前工作在实例
		self = f.f_locals[ 'self' ].__class__

		try:
			# 只有内置的class类型才有这个属性 __mro__, 比如tuple, object等
			mro = self.__mro__
		except AttributeError:
			stack = [self]
			mro = []
			while stack:
				curr = stack.pop(0)
				mro.append(curr)
				stack += curr.__bases__
		
		# 获得当前所在函数名
		funcName = f.f_code.co_name
		for c in mro:
			try:
				method = c.__dict__[ funcName ]
				if method.func_code == f.f_code:
					return c.__name__ + '.'
			except KeyError:
				pass
	except:
		return ""

def printMsg(keyword, args, isPrintPath):
	f = sys._getframe(2)

	# 打印文件名和行数
	#if isPrintPath:
	#	print f.f_code.co_filename + "(" + str(f.f_lineno) + ") :"

	#print "%s:%s%s: " % ( keyword, getClassName( f ), f.f_code.co_name ),
	for m in args:print (m)

def TRACE_MSG(*args): 
	printMsg("Trace:",   args, False)
	
def DEBUG_MSG(*args): 
	printMsg("Debug:",   args, True)
	
def INFO_MSG(*args): 
	printMsg("Info:",    args, False)
	
def WARNING_MSG(*args): 
	printMsg("Warning:", args, True)

def ERROR_MSG(*args): 
	printMsg("Error:",   args, True)

def HOOK_MSG(*args) :
	s = "HOOK_MSG: "

		