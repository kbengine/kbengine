KBEngine -- open source mmog server engine.
========

##Homepage
http://www.kbengine.org

##Releases
	sources		: https://github.com/kbengine/kbengine/releases 
	binarys		: https://sourceforge.net/projects/kbengine/files/

##Demo sources

	unity3d		: https://github.com/kbengine/kbengine/tree/master/kbe/src/client/unity3d
	ogre		: https://github.com/kbengine/kbengine/tree/master/kbe/src/client/ogre
	html5		: https://github.com/kbengine/kbengine/tree/master/kbe/src/client/html5


##Docs
	API		: https://github.com/kbengine/kbengine/tree/master/kbe/doc
	tutorial	: https://github.com/kbengine/kbengine/tree/master/tutorial
	online_docs	: http://www.kbengine.org/docs/

##Support
	Email		: admin@kbengine.org
	Maillist	: https://groups.google.com/forum/#!forum/kbengine_maillist

## QQ交流群
	群1		: 16535321
	群2		: 367555003

##What is KBEngine?

An open source mmog server engine, Using a simple protocol will be able to make the client and server interaction,
To use the KBEngine plugin quick combine with (unity3d, ogre, cocos2d, html5, etc..) to form a complete client.

Engine framework written using c + +, Game logic layer using Python, 
developers do not need to re-implement some common server-side technology,
Allows developers to concentrate on the game logic development, quickly create a variety of games.

(Frequently asked load limit, kbengine is designed to be multi-process distributed dynamic load balancing scheme, 
In theory only need to expand hardware can increase load limit, The complexity of the single machine 
load limit depends on the logic of the game itself.)

## 什么是KBEngine?
KBEngine是一款开源的游戏服务端引擎，使用简单的约定协议就能够使客户端与服务端进行交互，
使用KBEngine插件能够快速与(unity3d, ogre, cocos2d, html5, 等等.)技术结合形成一个完整的客户端。

服务端底层框架使用c++编写， 游戏逻辑层使用Python， 开发者无需重复的实现一些游戏服务端通用的底层技术，
将精力真正集中到游戏开发层面上来，快速的打造各种网络游戏。

(经常被问到承载上限, kbengine底层架构被设计为多进程分布式动态负载均衡方案， 理论上只需要不断扩展硬件就能够
不断增加承载上限, 单台机器的承载上限取决于游戏逻辑本身的复杂度。)

简单的介绍一下引擎的各个主要组件部分:


	· loginapp:
	client的登录验证, 验证通过则向客户端发放一个baseapp的地址， 之后客户端通过baseapp与服务端交互。
	可在多台机器部署多个loginapp进程来负载。 


	· dbmgr:
	实现数据的存取，默认使用mysql作为数据库。


	· baseappmgr:
	主要负责协调所有baseapp的工作，包括负载均衡处理等。


	· baseapp:
	客户端与服务端的交互只能通过loginapp分配的baseapp来完成。
	定时写entity的数据到数据库、 baseapp数据相互备份。
	可在多台机器部署多个baseapp进程来均衡负载。 


	· cellappmgr:
	主要负责协调所有cellapp的工作，包括负载均衡处理等。


	· cellapp:
	主要处理游戏实时逻辑， 如：aoi， navigate， ai, 休闲游戏房间内逻辑等等。
	可在多台机器部署多个cellapp进程来动态均衡负载。 


	· client:
	客户端我们将提供基础框架，这个框架不包括渲染部分和输入输出部分的具体实现, 
	我们将提供一个lib文件和一套API接口，开发者可以选择使用自己比较适合的图形渲染引擎与输入输出控制部分， 
	(目前已实现的ClientSDK版本: Unity3d、Ogre、Html5)


	· kbmachine:
	抽象出一个服务端硬件节点(一台硬件服务器只能存在一个这样的进程)。主要用途是接受远程指令处理本机上的组件启动与关闭, 
	提供本机上运行组件的接入口以及搜集当前机器上的一些信息， 
	如：CPU， 内存等。 这些信息会提供给一些对此比较感兴趣的组件。 


	· guiconsole: 
	这是一个控制台工具， 可以实时的观察服务端运行状态， 实时观测不同space中entity的动态，
	并支持动态调试服务端python逻辑层以及查看各个组件的日志，启动服务端与关闭等。 


	· messagelog: 
	收集和备份各个组件的运行日志，可使用guiconsole来查看和搜索相关信息。


##GO!


**设置环境变量:**

kbe会读取KBE_ROOT，KBE_RES_PATH， KBE_HYBRID_PATH系统环境变量来做一些事情。


Linux: (假设kbe被放置在~/目录下)

	[kbe@localhost ~]# vim ~/.bashrc

	ulimit -c unlimited

	export KBE_ROOT=~/kbengine/

	export KBE_RES_PATH=$KBE_ROOT/kbe/res/:$KBE_ROOT/demo/:$KBE_ROOT/demo/res/

	export KBE_HYBRID_PATH=$KBE_ROOT/kbe/bin/Hybrid64/

	[root@localhost ~]# vim /etc/passwd
	
	修改系统账号的uid必须唯一, uid用来区分不同的服务端组， 如果二台硬件维护一组服务端那么二台硬件上的uid必须一致, 
	值需大于0。
Windows:

	鼠标右键"我的电脑"->"高级"->"环境变量" 设置对应的值就好了。
	(注意: Windows下需要在环境变量中添加UID, 值需大于0)



KBE_ROOT:

	kbe根目录路径。


KBE_RES_PATH:

	相关资源路径用':'或者';'分隔, 第一个res必须是kbe引擎的res, 第二个res必须是用户脚本根目录， 其他无限制。


KBE_HYBRID_PATH:

	kbe二进制文件所在目录路径。



##编译:

linux:

	测试系统(x32&x64): centos >= 5.x, debian >= 5.x
	编译器 gcc: >= 4.4.x


	[root@localhost ~]# cd $KBE_ROOT/kbe/src

	[root@localhost /src]# make


windows:

	安装好vc2008sp1版本直接编译完即可

	KBE_ROOT\kbengine\kbe\src\kbengine_vs90.sln

注意: 

	1: 如使用其他版本编译器最好将openssl、log4cxx(kbe\src\libs\*.a)也重新编译。

	2: 某些平台上的mysql路径可能不是/usr/lib64/mysql/mysql_config

	修改kbe\src\build\common.mak中的MYSQL_CONFIG_PATH=/usr/lib64/mysql/mysql_config

	3: 在linux上编译完成如因为python无法初始化无法正常运行的情况(这是个bug http://bugs.python.org/issue11320):

	cd src\lib\python

	./configure

	make

	make install

	再启动服务端。


##配置数据库:
	1: 安装好mysql
		如果是windows系统则my.ini中加入如下代码使mysql大小写敏感
		[mysqld]
		lower_case_table_names=0

	2: 记得重启mysql服务， 否则不生效(命令行cmd输入):
		net stop mysql
		net start mysql

	3: 新建一个数据库， 假设数据库名为"kbe"
		mysql> create database kbe;

	4: 创建一个数据库账户， 假设用户名密码都为"kbe"
	    先删除匿名用户
		mysql> use mysql 
		mysql> delete from user where user=''; 
		mysql> FLUSH PRIVILEGES;

	    创建kbe用户
		mysql> grant all privileges on *.* to kbe@'%' identified by 'kbe';
		mysql> grant select,insert,update,delete,create,drop on *.* to kbe@'%' identified by 'kbe';
		mysql> FLUSH PRIVILEGES;

	    在CMD中测试一下是否能使用这个账号登陆mysql(请注意默认mysql端口为3306， 如不一致请修改kbengine_defs.xml->dbmgr-><port>330x</port>)， 
	    如果没有提示错误则账号配置完毕， 有错误请google
	    进入你的mysql安装目录找到mysql.exe所在目录, 然后cmd进入这个目录中执行如下语句:
	    C:\mysql\bin> mysql -ukbe -pkbe -hlocalhost -P3306

	5: 在res\server\kbengine_defs.xml的dbmgr节修改databaseName参数
		(推荐在demo\res\server\kbengine.xml重载修改)。
		如果mysql端口不是3306， 请在kbengine.xml中的dbmgr段加入<port>端口号</port>。



##启动服务端:


Linux:

	[root@localhost ~]# cd $KBE_HYBRID_PATH

	sh start.sh

	sh kill.sh

	(注意: 如有防火墙限制请设置防火墙规则对外开放这些TCP端口: loginapp登录端口、 baseapp登录端口具体请看
	kbengine.xml|kbengine_defs.xml。 以及UDP广播端口:20086-20088)
	(注意: 如果有二块网卡, 例如: eth0(公网ip)、eth1(局域网ip)
	请设置kbengine.xml|kbengine_defs.xml除baseapp|loginapp|billingsystem的externalInterface设置为eth0以外, 
	其他相关{internal|external}Interface为局域网ip的那块网卡(eth1), 并设置使用局域网ip来接收udp广播:
	/sbin/ip route del broadcast 255.255.255.255 dev eth0
	/sbin/ip route add broadcast 255.255.255.255 dev eth1
	)
Windows:

	cd KBE_HYBRID_PATH

	!(win)fixedstart.bat

	!(win)kill.bat

	或者使用tools\server\guiconsole\guiconsole.exe来启动和关闭服务端。
	(注意: Windows版本仅用于测试， 由于使用select并发socket处理性能有限。)

##日志:

	KBE_HYBRID_PATH目录下会产生各组件运行的日志信息"logs\*.log"。 
	也可以使用GUIConsole来查看， 但必须开启messagelog。

