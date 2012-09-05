mkdir docs
mkdir docs\html
copy /y Readme.html docs\html\index.html
copy /y doxygen.css docs\html\doxygen.css
xcopy /y /s images docs\html\images\
cd docs\html
"G:\Program Files\HTML Help Workshop\hhc.exe" index.hhp
cd ..\..
