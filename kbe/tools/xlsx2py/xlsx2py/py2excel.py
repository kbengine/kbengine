# -*- coding: utf-8 -*-
"""
*****************************************************************************************
    使用方法：python py2excel pyfile(utf-8)  readexcelfile, writeexcelfile
    pyfile请使用utf-8，不支持ANSI, py中的应有字典datas, allDatas(py文件至少有datas)
    readexcelfile为生成py文件的母表
    writeexcelfile 为导出的xlsx文件
*****************************************************************************************
"""

import sys
import os

import xlsxtool
from ExcelTool import ExcelTool

from config import *

class Sheet(object):
    """
    简表
    """
    @property
    def sheet(self):
        return self.__parentBook.getSheetByIndex(self.__index)

    def __init__(self, parentBook, sheetIndex):     
        self.__parentBook = parentBook
        self.__index =  sheetIndex

    def getIndex(self):
        return self.__index

    def getParentBook(self):
        return self.__parentBook

    def getColCount(self):
        return self.__parentBook.getColCount(self.__index)  
    
    def getColValues(self, col):
        return self.__parentBook.getColValues(self.sheet, col)

    def getRowValues(self, row):
        return self.__parentBook.getRowValues(self.sheet, row)


class py2excel(object):
    """
    
    """
    def __init__(self, pyfile, sourcefile, dstfile):
        """
        pyfile:py, sourcefile:source excel, excel:dest excel
        """
        self.pyfile = os.path.abspath(pyfile)
        if sourcefile == '':
            self.sourcefile = sourcefile
        else:
            self.sourcefile = os.path.abspath(sourcefile)

        self.dstfile = os.path.abspath(dstfile)
        self.xbook = None
        self.sheetCNames =  {}
        self.sheetENames = {}
        self.mapSheet = {}
        

####################导入py文件#######################
    def importPyModule(self):
        """
        import self.pyfile as python module
        """
        self.pyModule = None
        try:
            sys.path.append(PY_MODULE_PATH)
        except NameError:
            pass

        #try:
        pyPath, filename=  os.path.split(self.pyfile)
        pypos = filename.strip().rfind(".py")
        if pypos < 0:
            print( "pypypypypypypypy")
        else:
            filename = filename[:pypos]

        sys.path.append(pyPath)
        #try:
        self.pyModule = __import__(filename)
        #except:
            #print( 'import %s' %(self.pyfile))
            #sys.exit(1)
        sys.path.pop(-1)
        sys.path.pop(-1)

    def getSheetNameFromModule(self):
        if hasattr(self.pyModule, 'allDatas'):  
            return self.pyModule.allDatas
        else:
            return  None

############################从策划表读取信息#######################################
    def openXlsx(self):
        if xlsxtool.checkExtName(self.sourcefile, '.xlsx') or xlsxtool.checkExtName(self.sourcefile, ".xls"):
            self.xbook = ExcelTool(self.sourcefile)

            if not self.xbook.getWorkbookEx():
                print( "打开文件失败" )
                return

    def getSheetCNames(self):
        allDatas = self.getSheetNameFromModule()
        sheetCNames = {}
        for index in range(self.xbook.getSheetCount()):
            sheetName = self.xbook.getSheetNameByIndex(index)
            if sheetName.startswith(EXPORT_PREFIX_CHAR):
                if allDatas is  None:
                    sheetCNames[index] = sheetName
                elif sheetName[1:]  in allDatas:        #py文件编码认为是utf-8
                    sheetCNames[index] = sheetName
        
        if len(sheetCNames) == 0:
            print( 'no sheet' )
            self.xbook.close()
            sys.exit(1)
            
        if allDatas is None and len(sheetCNames) > 1:   #这个主要处理，没有allDatas的时候
            for k,v in sheetCNames.iteritems():
                print( "%d:%s"%(k,v) )

            while True:
                ii = raw_input('input your choice:')
                try:
                    ii = int(ii)
                except:
                    continue

                if ii > 0 and ii < len(sheetCNames):
                    print( sheetCNames[ii] )
                    self.sheetCNames[ii] = sheetCNames[ii]
                    break
        else:
            self.sheetCNames = sheetCNames


    def readXlsxHeader(self):
        """
        读取中英文对照
        """
        if self.xbook is None:
            print( "no file opened" )

        self.names = {}                                 #sn:表的中文名字,engName，chnName:字典key的英文(中文)名字，

        for si, sn in self.sheetCNames.items():     #chinese name of  sheetname, sheetindex
            sheet = Sheet(self.xbook, si)
            self.names[sn] = {}
            tmpEInt = 1
            tmpCInt = 1
            
            for (engStruct, chnName) in zip(sheet.getRowValues(EXPORT_DEFINE_ROW -1), sheet.getRowValues(EXPORT_DEFINE_ROW)):
                if engStruct is None:
                    continue
                if engStruct.find('['):
                    engName = engStruct[:engStruct.find('[')]
                else:
                    engName = 'undefineE_%d'%(tmpEInt,)
                    tmpEInt += 1

                if chnName is  None:
                    chnName = 'undefineC_%d'%(tmpCInt,)
                    tmpCInt += 1

                self.names[sn][engName] = chnName

        self.sheet = None
        self.xbook.close()      #覆盖的时候这是必须的
        
        self.xbook = None
        return self.names

    def writeNewXlsx(self):
        """
        py的字典写入到xlsx
        """
        def getWorkbook():
            dirs, filename = os.path.split(self.dstfile)        
            if not os.path.isdir(dirs):
                os.makedirs(dirs)
            return ExcelTool(self.dstfile) 
        
        if self.xbook is not None:
            self.xbook.close()
            self.xbook = None

        self.xbook = getWorkbook()
        
        self.xbook.getWorkbookEx(True)
        
        if self.sourcefile != '':
            self.writeXlsxWithC()
            
        else:
            self.writeXlsxWithoutC()        #没有中文


    def writeXlsxWithoutC(self):        #没有中文
        self.parseWriteSheet('datas')
        data  = None
        if hasattr(self.pyModule, 'datas'):
            data = self.pyModule.datas

        if data is None:
            return

        headerKeys = self.getHeaderKeys(data)
        self.newSheet = self.getWriteSheet('datas')
        self.writeXlsxHeader(headerKeys)
        self.writeData2Cells(data, headerKeys)

        self.xbook.close(saveChanges = True)
        

    def writeXlsxWithC(self):       #有中文的文件
        cnames = self.names.keys()
        self.parseWriteSheet(cnames)

        for cname, e2cDict in self.names.items():
            self.newSheet = self.getWriteSheet(cname)

            self.newSheet.UsedRange = None  #清空表的内容
            data = None

            if self.getSheetNameFromModule()  is not None:
                if  cname[1:].encode("utf-8") not  in self.getSheetNameFromModule():
                    continue
                else:
                    data = self.getSheetNameFromModule()[cname[1:].encode("utf-8")]

            elif hasattr(self.pyModule, 'datas'):
                data = self.pyModule.datas

            if data is None or not isinstance(data, dict):
                continue

            headerKeys = self.getHeaderKeys(data)
            headerCNames =  []

            for p, he in enumerate(headerKeys):
                cname = e2cDict.get(he, "py_%s"%(str(he),))
                headerCNames.append(cname)

            self.writeXlsxHeader(headerCNames)
            self.writeData2Cells(data, headerKeys)

        self.xbook.close(saveChanges = True)
    
    def writeXlsxHeader(self, headerCNames):
        """
        写到导出xlsx的第一行
        """
        for  pos, cn in enumerate(headerCNames):            #ANSI编码
            self.newSheet.cell(1, pos+1).value = cn

    def writeData2Cells(self, data, headerKeys):
        """
        字典的数据写入到excel中
        """

        if self.newSheet is None:
            return
        for vp, v in enumerate(data.values()):      #value include key
            for p, he in enumerate(headerKeys):
                text = self.convertType(v.get(he, ''))
                self.newSheet.cell(vp+2, p+1).value = text

        return

    def getHeaderKeys(self, data):
        headerKeys =  []
        for v in data.values(): #{1111:{'key':values,,}}
            for vk in v.keys():
                if vk not in headerKeys:
                    headerKeys.append(vk)

        return headerKeys

    def getWriteSheet(self, cname):
        """
        从workbook选取所要写入数据的sheet
        """
        if cname in self.repeatUse:
            newSheet = self.xbook.getSheetByIndex(self.repeatUse.pop(cname))

        elif len(self.useless) > 0:
            newSheet = self.xbook.getSheetByIndex(self.useless.pop(-1))
            newSheet.Name = cname
        else:
            newSheet = self.xbook.getXLSX().Sheets.Add()
            newSheet.Name = cname

        return newSheet

    def parseWriteSheet(self, cnames):
        """
        对即将要写的表做一些分析，保证一些表依旧存在
        """
        self.repeatUse = {} #表需要覆盖
        self.useless = []   #这些表被看做无用，需要新表的时候从这里取

        for index in  range(self.xbook.getSheetCount()):
            name = self.xbook.getSheetNameByIndex(index)
            if name in cnames:
                self.repeatUse[name] = index
            else:
                self.useless.append(index)
        return

    

    def convertType(self, val):
        """
        类型转换
        """
        if isinstance(val, bytes):
            return val.decode("utf-8")
        elif isinstance(val, (dict, list, tuple)):
            return xlsxtool.value_to_text(val)      
        return val

        
    def run(self):
        self.importPyModule()
        if self.sourcefile != '':
            self.openXlsx()
            self.getSheetCNames()
            self.readXlsxHeader()

        self.writeNewXlsx()
        
if __name__ == '__main__':
    if len(sys.argv[1:]) == 2: #没有中文表
        pyfile, dstfile = sys.argv[1:]
        a = py2excel(pyfile, '', dstfile)
        a.run()
        
    
    elif len(sys.argv[1:]) == 3:    #有中文表
        pyfile, sourcefile, dstfile  = sys.argv[1:]

        if False in map(lambda x:os.path.isfile(x), sys.argv[1:3]):
            print(  '文件呢?')
            sys.exit(1)

        a = py2excel(pyfile, sourcefile, dstfile)
        a.run()
        
    else:
        print( __doc__.decode("utf-8") )
        sys.exit(1)
    
    
        

