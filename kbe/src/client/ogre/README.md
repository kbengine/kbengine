client—ogre
=============

##官方网站:
http://www.kbengine.org

##GO!

**设置环境变量:**

kbe会读取KBE_ROOT，KBE_RES_PATH， KBE_HYBRID_PATH系统环境变量来做一些事情。

windows:

	鼠标右键"我的电脑"->"高级"->"环境变量" 设置对应的值就好了。

KBE_ROOT:

	kbe根目录路径。

KBE_RES_PATH:

	相关资源路径用':'或者';'分隔, 第一个res必须是kbe引擎的res, 第二个res必须是用户脚本根目录， 其他无限制。

KBE_HYBRID_PATH:

	kbe二进制文件所在目录路径。

##编译:

windows:

	1.下载安装OGRE 1.8.1 SDK for Visual C++ 2008(32-bit)，下载地址http://www.ogre3d.org/download/sdk。

	2.下载安装Microsoft DirectX 9.0。

	3.Microsoft Visual Studio下编译。

##运行：

windows:
	1.设置环境变量后编译运行exe即可。
