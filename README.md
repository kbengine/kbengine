kbengine
========

演示截图(点击图片看视频):
[![kbengine](https://github.com/downloads/kbengine/kbengine/demo.jpg)](http://v.youku.com/v_show/id_XMTc2MDcxMDUy.html)

**什么是KBEngine?**

kbengine仿照bigworld技术努力成为一款开源mmorpg引擎，bigworld引擎的特点是开发者无需接触c++底层，
无需重复性的实现网络，内存管理，线程管理等底层通用性技术，把精力集中到游戏开发层面上来，
使用python就能够简单高效的打造一款游戏。

先简单的介绍一下引擎的各个组件部分:

	· dbmgr:
	这个程序主要是用来处理游戏的数据库部分，它封装了mysql，能够很方便的完成各种数据库操作, 
	以及整个游戏的entityID分配等等, 共享数据(globaldata, baseAppData, cellAppData)。 


	· baseappmgr:
	主要是用来负责处理所有的baseapp的工作分配（负载平衡）等。服务器上会有一个或者多个baseapp，主要看使用者如何配置。 


	· baseapp:
	baseappmgr将一个client分配给它之后， 它才接受某个帐号登陆， 登陆后就会将client分配到一个合适的cellapp，
	一个帐号登陆到baseapp之后就不会再改变，这个baseapp会一直维护这个帐号，直到与他断开连接。当然baseapp还会处理很多的东西，
	例如entity需要存储到数据库的数据会定时给dbmgr处理， 备份entity cell部分的相关数据等等。客户端与服务器的通讯只能通过baseapp来完成, 
	它也充当服务器与客户端之间的安全墙。 


	· cellappmgr:
	负责所有cellapp的工作分配，包括负载均衡处理等。服务器上会有一个或者多个cellapp，主要看使用者如何配置。 


	· cellapp:
	一个cellapp负责处理一个或者多个space，当某个space消耗达到cellapp快无法承受时， 
	服务器会让比较轻松的cellapp与它一起分担消耗， 也就是说也会出现多个cellapp维护一个space。整个游戏的逻辑部分也在cellapp， 
	包括aoi, ai, entity移动等等。 


	· loginapp:
	它只处理client的登录排队与检查帐号， 帐号合法就会从baseappmgr得到一个baseapp的地址发给客户端，
	然后就与客户端断开连接。可在多台机器部署。 


	· client:
	客户端我们将提供基础框架，这个框架不包括渲染部分和输入输出部分的具体实现, 
	我们将提供一个lib文件和一套API接口，开发者可以使用自己比较擅长或者合适的图形渲染和输入输出控制部分， 
	当然我也会封装一套默认的相关模块，目前我们准备开源的渲染引擎ogre来实现图形表现部分。 


	· kbmachine:
	抽象出一个kbe硬件节点。主要用途是接受远程指令处理本机上的组件启动与关闭, 提供本机上运行组件的ip接口
	以及搜集当前机器上的一些信息， 如：CPU， 内存等。 这些信息会提供给一些对此比较感兴趣的组件。 


	· guiconsole: 
	控制工具， 可以实时网络连接到某个进程动态调试游戏， 启动服务器与关闭查看日志等。 


