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

signal.signal(signal.SIGINT, siginit)                       #Ctrl-c处理

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
    将excel数据导出为py文件 使用过程需要进行编码转换
    """
    def __init__(self, infile, outfile):
        sys.excepthook = xlsxError.except_hook                      #traceback处理,希望输出中文
        self.infile = os.path.abspath(infile)                       #暂存excel文件名
        self.outfile = os.path.abspath(outfile)                     #data文件名
        return

    def __initXlsx(self):
        self.xbook = ExcelTool(self.infile)

        while not self.xbook.getWorkbookEx():
            xlsxtool.exportMenu(EXPORT_INFO_RTEXCEL, OCallback = self.resetXlsx)

    def resetXlsx(self):
        """
        输入O(other)的回调
        关闭已打开的excel，然后重新打开
        """
        self.xbook.getWorkbookEx()

    def __initInfo(self):
        self.__exportSheetIndex = []        #存储可导表的索引
        self.headerDict = {}                #导出表第一行转为字典
        self.mapDict = {}                   #代对表生成的字典(第一行是代对表说明忽略)

#####################执行主题##########################
    def run(self):
        """
        带有$的列数据需要代对表,首先生成代对字典
        """
        self.__initXlsx()                       #初始excel相关
        self.__initInfo()                       #初始导表相关
        self.openFile()
        self.sth4Nth()                          #进入下一个阶段
        self.constructMapDict()                 #生成代对字典
        self.__onRun()

    def __onRun(self):
        self.writeLines  = 0                    #记录已写入的excel的行数
        self.parseDefineLine()                  #分析文件

###############寻找代对表和标记导入的表##################
    def sth4Nth(self):
        """
        something for nothing, 代对表和导入表需要有
        """
        for index in range(self.xbook.getSheetCount()):
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
        完毕
        """
        self.__exportSheetIndex.append(Eindex)

    def constructMapDict(self):
        """
        生成代对字典， 代对表只有一个
        """
        mapDict = {}
        sheet = self.xbook.getSheetByIndex(self.mapIndex)
        if not sheet:
            return

        for col in range(0, self.xbook.getRowCount(self.mapIndex)):
            colValues = self.xbook.getColValues(sheet, col)
            if colValues:
                for v in  [e for e in colValues[1:] if e and isinstance(e, str) and e.strip()]:
                    print (v)
                    mapStr = v.replace('：', ":")         #中文"："和":"
                    try:
                        k, v  = mapStr.split(":")
                        k = str.strip(k)
                        v = str.strip(v)
                        mapDict[k] = v
                    except Exception as errstr:
                        print( "waring：需要检查代对表 第%d列, err=%s"%(col , errstr))
        self.__onConstruct(mapDict)
        return

    def __onConstruct(self, mapDict):
        """
        代对字典生成完毕
        """
        self.mapDict = mapDict
        return

#####################文件头检测#######################
    def parseDefineLine(self):
        self.__checkDefine()        #检查定义是否正确
        self.__checkData()          #检查数据是否符合规则

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
        第一行的个元素是否符合定义格式"name[signs][func]"以及key是否符合规定
        """
        print(  "检测文件头(第一行)是否正确" )
        for index in self.__exportSheetIndex:
            self.sheetKeys = []
            headList = self.xbook.getRowValues(self.xbook.getSheetByIndex(index), EXPORT_DEFINE_ROW -1 )
            enName = []                                         #检查命名重复临时变量
            reTuples = []

            self.headerDict[index] = {}
            for c, head in enumerate(headList):
                if head is None or head.strip() == '':          #导出表的第一行None, 则这一列将被忽略
                    self.__onCheckSheetHeader(self.headerDict[index], c, None)
                    continue

                reTuple = self.__reCheck(head)

                if len(reTuple) == 3:                           #定义被分拆为三部分:name, signs, func,signs可以是空
                    name, signs, funcName = reTuple[0], reTuple[1][1:-1], reTuple[2][1:-1]
                    name = self.__convertKeyName(name)
                    for s in signs:                             #符号定义是否在规则之内
                        if s not in EXPORT_ALL_SIGNS:
                            self.xlsxClear(EXPORT_ERROR_NOSIGN, (EXPORT_DEFINE_ROW, c+1))

                    if EXPORT_SIGN_GTH in signs:                #是否为key
                        self.sheetKeys.append(c)

                    if len(self.sheetKeys) > EXPORT_KEY_NUMS:   #key是否超过规定的个数
                        self.xlsxClear(EXPORT_ERROR_NUMKEY, (EXPORT_DEFINE_ROW, c+1))

                    if name not in enName:                      #name不能重复
                        enName.append(name)
                    else:
                        self.xlsxClear(EXPORT_ERROR_REPEAT, \
                        (self.xbook.getSheetNameByIndex(index).encode(FILE_CODE), EXPORT_DEFINE_ROW, c+1))

                    if not hasFunc(funcName):                   #funcName是否存在
                        self.xlsxClear(EXPORT_ERROR_NOFUNC, (xlsxtool.toGBK(funcName), c+1))

                else:
                    self.xlsxClear(EXPORT_ERROR_HEADER, (self.xbook.getSheetNameByIndex(index).encode(FILE_CODE), EXPORT_DEFINE_ROW, c+1))

                self.__onCheckSheetHeader(self.headerDict[index], c, (name, signs, funcName))   #定义一行经常使用存起来了

            self.__onCheckDefine()

        return

    def __onCheckSheetHeader(self, DataDict, col, headerInfo):
        DataDict[col] = headerInfo

    def __onCheckDefine(self):
        if len(self.sheetKeys) != EXPORT_KEY_NUMS:                  #key也不能少
            self.xlsxClear(EXPORT_ERROR_NOKEY, ("需要%d而只有%d"%(EXPORT_KEY_NUMS,len(self.sheetKeys))))

        print( "文件头检测正确", time.ctime(time.time()) )

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
        列数据是否符合命名规范, 生成所需字典
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
                    rowval = self.xbook.getRowValues(sheet, row - 1)
                    childDict = {}
                    for col in range(1, cols + 1):
                        val = rowval[col - 1]
                        if val != None:
                            val = (str(rowval[col - 1]),)
                        else:
                            val = ("",)
                        #val = (self.xbook.getText(sheet, row, col),)
                        if self.headerDict[index][col-1] is None:
                            continue

                        name, sign, funcName = self.headerDict[index][col-1]
                        if '$' in sign and len(val[0]) > 0:
                            self.needReplace({'v':val[0], "pos":(row, col)})
                            v = self.mapDict[xlsxtool.GTOUC(xlsxtool.val2Str(val[0]))]  #mapDict:key是unicode.key都要转成unicode
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
                        #   try:
                        #       v = v.decode("gb2312").encode("utf-8")
                        #   except:
                        #       pass
                        childDict[name] = v

                    print( "当前:%i/%i" % (row, rows) )
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

##############符号字典的相关设置EXPORT_SIGN###################
    def isNotEmpty(self, cellData):
        if cellData['v'] is None:
            self.xlsxClear(EXPORT_ERROR_NOTNULL, (cellData['pos'], ))

    def needReplace(self, cellData):
        """宏替代"""
        v = cellData["v"].strip()

        if isinstance(v, float):    #防止数字报错(1:string) mapDict 是unicode字符串
            v = str(int(v))

        if v not in self.mapDict:   #检测而不替换
            self.xlsxClear(EXPORT_ERROR_NOTMAP, (cellData['pos'], v))

    def isKey(self, cellData):
        if not hasattr(self, "tempKeys"):
            self.tempKeys = []

        if cellData['v'] not in self.tempKeys:
            self.tempKeys.append(cellData['v'])
        else:
            self.xlsxClear(EXPORT_ERROR_REPKEY, (cellData['pos'], \
                (self.tempKeys.index(cellData['v'])+3, cellData['pos'][1] ), cellData['v']) )




###############export to  py部分######################
    def exportSheet(self):
        """
        导出
        """
        self.__onExportSheet()
        return

    def __onExportSheet(self):
        """
        数据转成py文件
        """
        self.writeXLSX2PY()
        return

    def openFile(self):
        """
        文件目录创建
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

        self.__onOpenFile(fileHandler)      #目录创建成功,文件打开
        return

    def __onOpenFile(self,  fileHandler):
        """
        py文件打开了,可以写文件了
        """
        self.fileName = self.outfile
        self.fileHandler = fileHandler
        del self.outfile

    def xlsxWrite(self, stream):
        """
        写入data文件
        """
        if not hasattr(self, "fileHandler"):
            self.xlsxClear(EXPORT_ERROR_FILEOPEN, ())
        try:
            self.fileHandler.write(stream)
        except Exception as errstr:
            self.xlsxClear(EXPORT_ERROR_IOOP, (errstr))

    def writeXLSX2PY(self):
        """
        文件 前几行文字
        """
        self.writeBody()
        return

    def writeHead(self):
        print( "开始写入文件:", time.ctime(time.time()) )
        try:
            SheetName = self.xbook.getSheetNameByIndex(self.curProIndex[-1])
        except:
            print( "获取表的名字出错" )

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
        #   xlsxError.info_input(EXPORT_INFO_ING, (self.xbook.getSheetNameByIndex(index).encode(FILE_CODE), ))
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
        文件尾
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
        print( "写完了time:", time.ctime(time.time()) )

##############其他##################
    def xlsxClose(self):
        """
        关闭文档
        """
        if hasattr(self, "fileHandler"):
            self.fileHandler.close()

        self.xbook.close()
        return

    def xlsxClear(self, errno = 0, msg = ''):
        """
        程序异常退出清理打开的Excel
        """
        self.xlsxClose()
        if errno > 0:
            raise xlsxError.xe(errno, msg)
        else:
            sys.exit(1)

    def xlsxbyebye(self):
        """
        正常退出
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
    使用方法：
    python xlsx2py excelName.xls(x) data.py
    """
    try:
        outfile = sys.argv[1]
    except:
        print( main.__doc__ )
        return
    
    for infile in sys.argv[2:]:
        print( "开始导表:[%s] max=%i" % (infile, len(sys.argv[2:])) )
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
