# -*- coding: gb2312 -*-
"""
配置文件
"""


#PY_MODULE_PATH = r'res\entities\datum'
######生成数据的表头设置##########
EXPORT_DATA_CODING = "utf-8"


EXPORT_DATA_HEAD = "# -*- coding: %s -*-\n\n"%(EXPORT_DATA_CODING,)



###############################
#常量
###############################
#导出sheet前缀
EXPORT_PREFIX_CHAR = '@'
EXPORT_DEFINE_ROW = 1

EXPORT_KEY_NUMS = 1

MAP_DEFINE_ROW = 1
MAP_DATA_ROW = 3

#代对表的名字
EXPORT_MAP_SHEET = '代对表='

#文件编码，导出文件的编码设为UTF-8
FILE_CODE = "GB2312"

############################
#命令相关				   #
############################
EXPORT_SIGN_DOT = '.'
EXPORT_SIGN_DOLLAR = '$'
EXPORT_SIGN_GTH = '!'

CHECK_FUN = None

#format: index:checkfunc
EXPORT_SIGN= {
	EXPORT_SIGN_DOT:	CHECK_FUN,
	EXPORT_SIGN_DOLLAR : 	CHECK_FUN,
	EXPORT_SIGN_GTH:	CHECK_FUN,
}

EXPORT_ALL_SIGNS = [e for e in EXPORT_SIGN.keys()]

####################error字典##########################
EXPORT_ERROR_NOSHEET = 1
EXPORT_ERROR_NOMAP = 2
EXPORT_ERROR_HEADER = 3
EXPORT_ERROR_NOTNULL = 4
EXPORT_ERROR_REPEAT = 5
EXPORT_ERROR_REPKEY = 6
EXPORT_ERROR_NUMKEY = 7
EXPORT_ERROR_NOKEY = 8
EXPORT_ERROR_NOFUNC = 9
#数据检测错误
EXPORT_ERROR_DATAINV  = 20
EXPORT_ERROR_NOSIGN = 21
EXPORT_ERROR_NOTMAP = 22
EXPORT_ERROR_FUNC	= 23
#文件IO操作
EXPORT_ERROR_CPATH = 30
EXPORT_ERROR_FILEOPENED = 31
EXPORT_ERROR_NOEXISTFILE = 32
EXPORT_ERROR_OTHER = 101
EXPORT_ERROR_FILEOPEN = 102
EXPORT_ERROR_IOOP = 103

EXPORT_ERROR = {
	EXPORT_ERROR_NOSHEET:'无表可导',
	EXPORT_ERROR_NOMAP:'无代对表',
	EXPORT_ERROR_HEADER:'文件头错误',
	EXPORT_ERROR_NOTNULL:'不能为空',
	EXPORT_ERROR_REPEAT:'命名重复',
	EXPORT_ERROR_DATAINV:'数据与定义不符合',
	EXPORT_ERROR_OTHER:'就是错误',
	EXPORT_ERROR_NUMKEY:'需要的key太多',
	EXPORT_ERROR_NOSIGN:'不存在的符号',
	EXPORT_ERROR_REPKEY:'作为关键字的列有重复的key值',
	EXPORT_ERROR_NOTMAP:'需要代对，而没有代对关系',
	EXPORT_ERROR_NOKEY:'没有主key',
	EXPORT_ERROR_CPATH:'目录创建失败',
	EXPORT_ERROR_FILEOPENED:"文件已打开请关闭后，在运行",
	EXPORT_ERROR_NOFUNC:"不存在的转化函数",
	EXPORT_ERROR_NOEXISTFILE:'excel文件不存在',
	EXPORT_ERROR_FILEOPEN:'文件打开失败',
	EXPORT_ERROR_IOOP:'文件读写错误',
	EXPORT_ERROR_FUNC:'函数错误',
}

EXPORT_INFO_NULL = 0
EXPORT_INFO_OK = 1
EXPORT_INFO_ING = 2
EXPORT_INFO_CDIR = 3
EXPORT_INFO_YN = 4
EXPORT_INFO_RTEXCEL = 5

EXPORT_INFO = {
	EXPORT_INFO_NULL:"\b",
	EXPORT_INFO_YN:"是否继续Y or N",
	EXPORT_INFO_OK:"文件配置正确，是否要导入(Y or N)",
	EXPORT_INFO_ING:"正在导表",
	EXPORT_INFO_CDIR:"文件已打开",
	EXPORT_INFO_RTEXCEL:'关闭文件后重试， 你可以输入O让程序帮你关闭',
}
