# -*- coding: gb2312 -*-
# written by kebiao, 2010/08/20

from win32com.client import Dispatch
import os
import sys

class ExcelTool:
	"""
	简单的封装excel各种操作
	系统要求， windows系统， 安装python2.6以及pywin32-214.win32-py2.6.exe, 以及ms office
	"""
	def __init__(self, fileName):
		#try:
		#	self.close()
		#except:
		#	pass

		self.__xapp = Dispatch("Excel.Application")

		self.__xlsx = None

		self.fileName = os.path.abspath(fileName)

	def getWorkbook(self, forcedClose = False):
		"""
		如果Workbook已经打开需要先关闭后打开
		forcedClose：是否强制关闭，后打开该Workbook
		"""
		try:
			wn  = len(self.__xapp.Workbooks)
		except:
			print('程序异常退出，这可能是你打开编辑了"某文件"而没有保存该文件造成的，请保存该文件')
			sys.exit(1)

		for x in range(0, wn):
			Workbook = self.__xapp.Workbooks[x]

			if self.fileName == os.path.join(Workbook.Path, Workbook.name):
				if forcedClose:
					Workbook.Close(SaveChanges = False)
				return False

		self.__xlsx = self.__xapp.Workbooks.Open(self.fileName)			#打开文件
		return True

	def getXApp(self):
		return self.__xapp

	def getXLSX(self):
		return self.__xlsx

	def close(self, saveChanges = False):
		"""
		关闭excel应用
		"""
		if self.__xapp:
			self.__xlsx.Close(SaveChanges = saveChanges)
			if len(self.__xapp.Workbooks) ==0:
				self.__xapp.Quit()
		else:
			return False

	def getSheetCount(self):
		"""
		获得工作表个数
		"""
		return self.__xlsx.Sheets.Count

	def getSheetNameByIndex(self, index):
		"""
		获得excel上指定索引位置上的表名称
		"""
		return self.getSheetByIndex(index).Name

	def getSheetByIndex(self, index):
		"""
		获得excel上指定索引位置上的表
		"""
		if index in range(1, len(self.__xlsx.Sheets)+1):
			return self.__xlsx.Sheets(index)

		else:
			return None

	def getRowCount(self, sheetIndex):
		"""
		获得一排有多少元素
		"""
		return self.getSheetByIndex(sheetIndex).Cells(1).CurrentRegion.Columns.Count

	def getColCount(self, sheetIndex):
		"""
		获得一列有多少元素
		"""
		return self.getSheetByIndex(sheetIndex).Cells(1).CurrentRegion.Rows.Count

	def getValue(self, sheet, row, col):
		"""
		获得某个工作表的某个位置上的值
		"""
		return sheet.Cells(row, col).Value

	def getText(self, sheet, row, col):
		"""
		获得某个工作表的某个位置上的值
		"""
		return sheet.Cells(row, col).Text

	def getRowValues(self, sheet, row):
		"""
		整排
		"""
		return sheet.Cells(1).CurrentRegion.Rows[row].Value[0]

	def getSheetRowIters(self, sheet, row):
		"""
		行迭代器
		"""
		return sheet.Cells(1).CurrentRegion.Rows

	def getSheetColIters(self, sheet, col):
		"""
		列迭代器
		"""
		return sheet.Cells(1).CurrentRegion.Columns

	def getColValues(self, sheet, col):
		"""
		整列
		"""
		return sheet.Cells(1).CurrentRegion.Columns[col].Value

#---------------------------------------------------------------------
#   使用例子
#---------------------------------------------------------------------
def main():
	xbook = ExcelTool("d:\\test1.xlsx")

	print("sheetCount=%i" % xbook.getSheetCount())

	for x in range(1, xbook.getSheetCount() +1 ):
	   print( "      ", xbook.getSheetNameByIndex(x))

	print( "sheet1:rowCount=%i, colCount=%i" % (xbook.getRowCount(1), xbook.getColCount(1)))

	for r in range(1, xbook.getRowCount(1) + 1):
		for c in range(1, xbook.getColCount(1) + 1):
			val = xbook.getValue(xbook.getSheetByIndex(2), r, c)
			if val:
				print( "DATA:", val)

if __name__ == "__main__":
	main()




