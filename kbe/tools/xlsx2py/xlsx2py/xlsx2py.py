# -*- coding: gb2312 -*-
import sys
import re
import os
import string
import signal
import time
import codecs
import json

from ExcelTool import ExcelTool
from config import *
import functions
try:
	import character
except:
	character = functions
import xlsxtool
import xlsxError
import copy

SYS_CODE = sys.getdefaultencoding()

def siginit(sigNum, sigHandler):
	print("byebye")
	sys.exit(1)

signal.signal(signal.SIGINT, siginit)   					#Ctrl-c����

def hasFunc(funcName):
	return hasattr(character, funcName) or hasattr(functions, funcName)

def getFunc(funcName):
	if hasattr(character, funcName):
		return getattr(character, funcName)
	return getattr(functions, funcName)

g_dctDatas = {}
g_fdatas = {}

class xlsx2py(object):
	"""
	��excel���ݵ���Ϊpy�ļ� ʹ�ù�����Ҫ���б���ת��
	"""
	def __init__(self, infile, outfile):
		sys.excepthook = xlsxError.except_hook						#traceback����,ϣ���������
		self.infile = os.path.abspath(infile)						#�ݴ�excel�ļ���
		self.outfile = os.path.abspath(outfile)						#data�ļ���
		return

	def __initXlsx(self):
		self.xbook = ExcelTool(self.infile)

		while not self.xbook.getWorkbook(forcedClose = True):
			xlsxtool.exportMenu(EXPORT_INFO_RTEXCEL, OCallback = self.resetXlsx)

	def resetXlsx(self):
		"""
		����O(other)�Ļص�
		�ر��Ѵ򿪵�excel��Ȼ�����´�
		"""
		self.xbook.getWorkbook(forcedClose = True)

	def __initInfo(self):
		self.__exportSheetIndex = []		#�洢�ɵ��������
		self.headerDict = {} 				#�������һ��תΪ�ֵ�
		self.mapDict = {} 					#���Ա����ɵ��ֵ�(��һ���Ǵ��Ա�˵������)

#####################ִ������##########################
	def run(self):
		"""
		����$����������Ҫ���Ա�,�������ɴ����ֵ�
		"""
		self.__initXlsx()						#��ʼexcel���
		self.__initInfo()						#��ʼ�������
		self.openFile()
		self.sth4Nth()							#������һ���׶�
		self.constructMapDict()					#���ɴ����ֵ�
		self.__onRun()

	def __onRun(self):
		self.writeLines  = 0					#��¼��д���excel������
		self.parseDefineLine()					#�����ļ�

###############Ѱ�Ҵ��Ա�ͱ�ǵ���ı�##################
	def sth4Nth(self):
		"""
		something for nothing, ���Ա�͵������Ҫ��
		"""
		for index in range(1, self.xbook.getSheetCount() + 1):
			sheetName = self.xbook.getSheetNameByIndex(index)
			if sheetName == EXPORT_MAP_SHEET:
				self.__onFindMapSheet(index)
			if sheetName.startswith(EXPORT_PREFIX_CHAR):
				self.__onFindExportSheet(index)
		self.onSth4Nth()

	def onSth4Nth(self):
		"""
		"""
		if not hasattr(self, 'mapIndex'):
			self.xlsxClear(EXPORT_ERROR_NOMAP)

		if len(self.__exportSheetIndex) == 0:
			xlsxError.error_input(EXPORT_ERROR_NOSHEET)

		return

	def __onFindMapSheet(self, mapIndex):
		self.mapIndex = mapIndex
		return

	def __onFindExportSheet(self, Eindex):
		"""
		���
		"""
		self.__exportSheetIndex.append(Eindex)

	def constructMapDict(self):
		"""
		���ɴ����ֵ䣬 ���Ա�ֻ��һ��
		"""
		mapDict = {}
		sheet = self.xbook.getSheetByIndex(self.mapIndex)
		if not sheet:
			return

		for col in range(0, self.xbook.getRowCount(self.mapIndex)):
			colValues = self.xbook.getColValues(sheet, col)
			if colValues:
				for v in  [e for e in colValues[1:] if e[0] and isinstance(e[0], str) and e[0].strip()]:
					print (v)
					mapStr = v[0].replace('��', ":")			#����"��"��":"
					try:
						k, v  = mapStr.split(":")
						k = str.strip(k)
						v = str.strip(v)
						mapDict[k] = v
					except Exception as errstr:
						print( "waring����Ҫ�����Ա� ��%d��, err=%s"%(col , errstr))
		self.__onConstruct(mapDict)
		return

	def __onConstruct(self, mapDict):
		"""
		�����ֵ��������
		"""
		self.mapDict = mapDict
		return

#####################�ļ�ͷ���#######################
	def parseDefineLine(self):
		self.__checkDefine()		#��鶨���Ƿ���ȷ
		self.__checkData()			#��������Ƿ���Ϲ���

	def __reCheck(self, head):
		pattern = "(\w+)(\[.*])(\[\w+\])"
		reGroups =re.compile(pattern).match(head)

		if not reGroups:
			return ()
		return reGroups.groups()

	def __convertKeyName(self, name):
		try:
			tname = eval(name)
		except:
			pass
		else:
			if type(tname) == int or type(tname) == float:
				return tname

		return name

	def __checkDefine(self):
		"""
		��һ�еĸ�Ԫ���Ƿ���϶����ʽ"name[signs][func]"�Լ�key�Ƿ���Ϲ涨
		"""
		print(  "����ļ�ͷ(��һ��)�Ƿ���ȷ" )
		for index in self.__exportSheetIndex:
			self.sheetKeys = []
			headList = self.xbook.getRowValues(self.xbook.getSheetByIndex(index), EXPORT_DEFINE_ROW -1 )
			enName = []											#��������ظ���ʱ����
			reTuples = []

			self.headerDict[index] = {}
			for c, head in enumerate(headList):
				if head is None or head.strip() == '':			#������ĵ�һ��None, ����һ�н�������
					self.__onCheckSheetHeader(self.headerDict[index], c, None)
					continue

				reTuple = self.__reCheck(head)

				if len(reTuple) == 3:							#���屻�ֲ�Ϊ������:name, signs, func,signs�����ǿ�
					name, signs, funcName = reTuple[0], reTuple[1][1:-1], reTuple[2][1:-1]
					name = self.__convertKeyName(name)
					for s in signs:								#���Ŷ����Ƿ��ڹ���֮��
						if s not in EXPORT_ALL_SIGNS:
							self.xlsxClear(EXPORT_ERROR_NOSIGN, (EXPORT_DEFINE_ROW, c+1))

					if EXPORT_SIGN_GTH in signs:				#�Ƿ�Ϊkey
						self.sheetKeys.append(c)

					if len(self.sheetKeys) > EXPORT_KEY_NUMS:	#key�Ƿ񳬹��涨�ĸ���
						self.xlsxClear(EXPORT_ERROR_NUMKEY, (EXPORT_DEFINE_ROW, c+1))

					if name not in enName:						#name�����ظ�
						enName.append(name)
					else:
						self.xlsxClear(EXPORT_ERROR_REPEAT, \
						(self.xbook.getSheetNameByIndex(index).encode(FILE_CODE), EXPORT_DEFINE_ROW, c+1))

					if not hasFunc(funcName):					#funcName�Ƿ����
						self.xlsxClear(EXPORT_ERROR_NOFUNC, (xlsxtool.toGBK(funcName), c+1))

				else:
					self.xlsxClear(EXPORT_ERROR_HEADER, (self.xbook.getSheetNameByIndex(index).encode(FILE_CODE), EXPORT_DEFINE_ROW, c+1))

				self.__onCheckSheetHeader(self.headerDict[index], c, (name, signs, funcName))	#����һ�о���ʹ�ô�������

			self.__onCheckDefine()

		return

	def __onCheckSheetHeader(self, DataDict, col, headerInfo):
		DataDict[col] = headerInfo

	def __onCheckDefine(self):
		if len(self.sheetKeys) != EXPORT_KEY_NUMS:					#keyҲ������
			self.xlsxClear(EXPORT_ERROR_NOKEY, ("��Ҫ%d��ֻ��%d"%(EXPORT_KEY_NUMS,len(self.sheetKeys))))

		print( "�ļ�ͷ�����ȷ", time.ctime(time.time()) )

	def sheetIndex2Data(self):
		self.sheet2Data = {}
		for index in self.__exportSheetIndex:
			SheetName = self.xbook.getSheetNameByIndex(index)
			sheetName = SheetName[SheetName.find(EXPORT_PREFIX_CHAR)+1:]
			if sheetName in self.mapDict:
				dataName = self.mapDict[sheetName]
				if dataName in self.sheet2Data:
					self.sheet2Data[dataName].append(index)
				else:
					self.sheet2Data[dataName] =  [index]

	def __checkData(self):
		"""
		�������Ƿ���������淶, ���������ֵ�
		"""
		self.sheetIndex2Data()
		self.dctDatas = g_dctDatas
		self.hasExportedSheet = []

		for dataName, indexList  in self.sheet2Data.items():
			self.curIndexMax = len(indexList)
			self.curProIndex = []
			for index in indexList:
				sheet = self.xbook.getSheetByIndex(index)
				self.curProIndex.append(index)

				cols =  self.xbook.getRowCount(index)
				rows  = self.xbook.getColCount(index)
				if dataName not in self.dctDatas:
					self.dctDatas[dataName] = {}
				self.dctData = self.dctDatas[dataName]

				for row in range(3,  rows + 1):
					childDict = {}
					for col in range(1, cols + 1):
						val = (self.xbook.getText(sheet, row, col),)
						if self.headerDict[index][col-1] is None:
							continue

						name, sign, funcName = self.headerDict[index][col-1]
						if '$' in sign and len(val[0]) > 0:
							self.needReplace({'v':val[0], "pos":(row, col)})
							v = self.mapDict[xlsxtool.GTOUC(xlsxtool.val2Str(val[0]))]  #mapDict:key��unicode.key��Ҫת��unicode
						else:
							v = val[0]
						if EXPORT_SIGN_DOT in sign and v is None:
							self.xlsxClear(EXPORT_ERROR_NOTNULL, (col, row))

						try:
							sv = v#xlsxtool.toGBK(v)
						except:
							sv = v

						func = getFunc(funcName)

						try:
							v = func(self.mapDict, self.dctData, childDict, sv)
						except Exception as errstr:
							self.xlsxClear(EXPORT_ERROR_FUNC, (errstr, funcName, sv, row, col))
							
						for ss in sign.replace('$',''):
							EXPORT_SIGN[ss](self,{"v":v,"pos":(row, col)})

						#if isinstance(v, (isinstance, unicode)):
						#	try:
						#		v = v.decode("gb2312").encode("utf-8")
						#	except:
						#		pass
						childDict[name] = v

					print( "��ǰ:%i/%i" % (row, rows) )
					self.dctData[self.tempKeys[-1]] = copy.deepcopy(childDict)

				self.writeHead()

			overFunc = self.mapDict.get('overFunc')
			if overFunc is not None:
				func = getFunc(overFunc)
				self.dctData = func(self.mapDict, self.dctDatas, self.dctData, dataName)
				self.dctDatas[dataName] = self.dctData
			
			g_dctDatas.update(self.dctDatas)
			self.__onCheckSheet()
			
		self.__onCheckData()
		self.writeFoot()

	def __onCheckSheet(self):
		if hasattr(self, "tempKeys"):
			del self.tempKeys
		return

	def __onCheckData(self):
		self.exportSheet()

##############�����ֵ���������EXPORT_SIGN###################
	def isNotEmpty(self, cellData):
		if cellData['v'] is None:
			self.xlsxClear(EXPORT_ERROR_NOTNULL, (cellData['pos'], ))

	def needReplace(self, cellData):
		"""�����"""
		v = cellData["v"].strip()

		if isinstance(v, float):	#��ֹ���ֱ���(1:string) mapDict ��unicode�ַ���
			v = str(int(v))

		if v not in self.mapDict:	#�������滻
			self.xlsxClear(EXPORT_ERROR_NOTMAP, (cellData['pos'], v))

	def isKey(self, cellData):
		if not hasattr(self, "tempKeys"):
			self.tempKeys = []

		if cellData['v'] not in self.tempKeys:
			self.tempKeys.append(cellData['v'])
		else:
			self.xlsxClear(EXPORT_ERROR_REPKEY, (cellData['pos'], \
				(self.tempKeys.index(cellData['v'])+3, cellData['pos'][1] ), cellData['v']) )




###############export to  py����######################
	def exportSheet(self):
		"""
		����
		"""
		self.__onExportSheet()
		return

	def __onExportSheet(self):
		"""
		����ת��py�ļ�
		"""
		self.writeXLSX2PY()
		return

	def openFile(self):
		"""
		�ļ�Ŀ¼����
		"""
		dirPath = os.path.split(self.outfile)[0]

		if not os.path.isdir(dirPath):
			try:
				xlsxtool.createDir(dirPath)
			except:
				self.xlsxClear(EXPORT_ERROR_CPATH, (dirPath, ))
		try:
			fileHandler = codecs.open(self.outfile, "w+",'utf-8')
			#fileHandler = open(self.outfile, "w+")
		except:
			self.xlsxClear(EXPORT_ERROR_FILEOPEN, (self.outfile, ))

		self.__onOpenFile(fileHandler)		#Ŀ¼�����ɹ�,�ļ���
		return

	def __onOpenFile(self,  fileHandler):
		"""
		py�ļ�����,����д�ļ���
		"""
		self.fileName = self.outfile
		self.fileHandler = fileHandler
		del self.outfile

	def xlsxWrite(self, stream):
		"""
		д��data�ļ�
		"""
		if not hasattr(self, "fileHandler"):
			self.xlsxClear(EXPORT_ERROR_FILEOPEN, ())
		try:
			self.fileHandler.write(stream)
		except Exception as errstr:
			self.xlsxClear(EXPORT_ERROR_IOOP, (errstr))

	def writeXLSX2PY(self):
		"""
		�ļ� ǰ��������
		"""
		self.writeBody()
		return

	def writeHead(self):
		print( "��ʼд���ļ�:", time.ctime(time.time()) )
		try:
			SheetName = self.xbook.getSheetNameByIndex(self.curProIndex[-1])
		except:
			print( "��ȡ������ֳ���" )

		sheetName = SheetName[SheetName.find(EXPORT_PREFIX_CHAR)+1:]
		if sheetName in self.mapDict:
			dataName = self.mapDict[sheetName]
			self.hasExportedSheet.append(self.curProIndex[-1])
		else:
			self.xlsxClear(2, (sheetName.encode(FILE_CODE),))

		stream  = ""
		dataFileInfo = (self.infile + '.' + SheetName).encode("UTF-8")

		if len(self.hasExportedSheet) <= 1:
			stream =  EXPORT_DATA_HEAD
			globalDefs = self.mapDict.get('globalDefs', '')
			if len(globalDefs) > 0:
				func = getFunc(globalDefs)
				globalDefs = func(self.dctData)
				if len(globalDefs) > 0:
					globalDefs += "\n"
					if "globalDefs" in g_fdatas:
						g_fdatas["globalDefs"] += globalDefs
					else:
						g_fdatas["globalDefs"] = globalDefs


	def writeBody(self):
		#for index  in self.curProIndex:
		#	xlsxError.info_input(EXPORT_INFO_ING, (self.xbook.getSheetNameByIndex(index).encode(FILE_CODE), ))
		self.xlsxWrite(EXPORT_DATA_HEAD)
		if "globalDefs" in g_fdatas:
			self.xlsxWrite(g_fdatas["globalDefs"])

		for dataName, datas in g_dctDatas.items():
			stream = dataName + "="
			#stream += xlsxtool.dict_to_text(datas) + "\n"
			stream += "%s\n" % (datas)
			self.xlsxWrite(stream)
			jsonhandle = codecs.open(self.fileHandler.stream.name + "." + dataName + ".json", "w+",'utf-8')
			s = json.dumps(datas)
			jsonhandle.write("{%s}" % (s[1:-1]))
			jsonhandle.close()
			
	def writeFoot(self):
		"""
		�ļ�β
		"""
		if len(self.hasExportedSheet) < len(self.__exportSheetIndex):
			return

		allDataDefs = self.mapDict.get('allDataDefs', '')
		if len(allDataDefs) > 0:
			func = getFunc(allDataDefs)
			allDataDefs = func(self.dctData)
			if "allDataDefs" in g_fdatas:
					g_fdatas["allDataDefs"] += allDataDefs
			else:
					g_fdatas["allDataDefs"] = allDataDefs

		stream = "\nallDatas = {\n"
		for dataName, indexList in self.sheet2Data.items():
			for index in indexList:
				SheetName = self.xbook.getSheetNameByIndex(index)
				sheetName = SheetName[SheetName.find(EXPORT_PREFIX_CHAR)+1:]
				stream += "\t'" +  sheetName
				stream += "':"
				stream += dataName
				stream += ",\n"

		if len(allDataDefs) > 0:
			stream += "\t" + g_fdatas["allDataDefs"] + ",\n"

		stream +="}"
		self.xlsxWrite(stream)
		self.xlsxbyebye()
		print( "д����time:", time.ctime(time.time()) )

##############����##################
	def xlsxClose(self):
		"""
		�ر��ĵ�
		"""
		if hasattr(self, "fileHandler"):
			self.fileHandler.close()

		self.xbook.close()
		return

	def xlsxClear(self, errno = 0, msg = ''):
		"""
		�����쳣�˳�����򿪵�Excel
		"""
		self.xlsxClose()
		if errno > 0:
			raise xlsxError.xe(errno, msg)
		else:
			sys.exit(1)

	def xlsxbyebye(self):
		"""
		�����˳�
		"""
		self.xlsxClose()
		return

	def getSheetsCounts(self):
		return reduce(lambda x,y:x+y, \
			[self.xbook.getColCount(index) for index in self.__exportSheetIndex])

EXPORT_SIGN['.'] = xlsx2py.isNotEmpty
EXPORT_SIGN['$'] = xlsx2py.needReplace
EXPORT_SIGN['!'] = xlsx2py.isKey

def main():
	"""
	ʹ�÷�����
	python xlsx2py excelName.xls(x) data.py
	"""
	try:
		outfile = sys.argv[1]
	except:
		print( main.__doc__ )
		return
	
	for infile in sys.argv[2:]:
		print( "��ʼ����:[%s] max=%i" % (infile, len(sys.argv[2:])) )
		if os.path.isfile(infile):
			a = xlsx2py(infile, outfile)
			xlsxtool.exportMenu(EXPORT_INFO_OK)
			a.run()
		else:
			xlsxError.error_input(EXPORT_ERROR_NOEXISTFILE, (infile,))
		print( '-------------------------------THE END------------------------------------------------' )
	
	sys.exit()
	
if __name__ == '__main__':
	main()
