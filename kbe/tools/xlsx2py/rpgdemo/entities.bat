call basePath.bat
@echo off
set pydatas=%ktpydatas%/d_entities.py
set excel1=%ktexcels%/xlsxs/entities.xlsx
echo on
python ../xlsx2py/xlsx2py.py %pydatas% %excel1%
if not defined ktall (ping -n 30 127.1>nul)

