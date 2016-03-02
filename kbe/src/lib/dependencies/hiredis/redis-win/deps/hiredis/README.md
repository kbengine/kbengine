hiredis-win
===========

hiredis库的windows版本，也可在linux下编译，跨平台

基于官方最后一个release版本(0.11.0)进行修改，在windows上实现hiredis所有的功能(基于官方版本的test.c文件所有测试用例全部通过)

编译步骤:

  1 使用vs2010打开./vsproject/hiredis-win.sln
  
  2 编译hiredis-win-lib，项目默认配置为MD版本，若有别的需求可自行修改工程配置。
  
  3 编译生成的库文件在该文件夹下的debug或release文件夹下。
