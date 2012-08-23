call basePath.bat
@echo off
set pydatas=%ktpydatas%/avatar_inittab.py
set excel1=%ktexcels%/基础表格设计/角色基础属性表.xlsx
echo on
python xlsx2py/xlsx2py.py %pydatas% %excel1%
if not defined ktall (ping -n 30 127.1>nul)
