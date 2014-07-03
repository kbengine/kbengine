KBEngine -- open source MMOG server engine.
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
	Email		: kbengine_maillist@googlegroups.com
	Maillist	: https://groups.google.com/d/forum/kbengine_maillist

## QQ交流群
	16535321

##What is KBEngine?

An open source MMOG server engine, Using a simple protocol will be able to make the client and server interaction,
To use the KBEngine-plugins quick combine with (Unity3D, OGRE, Cocos2d, HTML5, etc.) to form a complete client.

Engine framework written using c++, Game logic layer using Python, 
developers do not need to re-implement some common server-side technology,
Allows developers to concentrate on the game logic development, quickly create a variety of games.

(Frequently asked load-limit, kbengine is designed to be multi-process distributed dynamic load balancing scheme, 
In theory only need to expand hardware can increase load-limit, The complexity of the single machine 
load-limit depends on the logic of the game itself.)

## 什么是KBEngine?
一款开源的游戏服务端引擎，使用简单的约定协议就能够使客户端与服务端进行交互，
使用KBEngine插件能够快速与(Unity3D, OGRE, Cocos2d, HTML5, 等等)技术结合形成一个完整的客户端。

服务端底层框架使用c++编写， 游戏逻辑层使用Python， 开发者无需重复的实现一些游戏服务端通用的底层技术，
将精力真正集中到游戏开发层面上来，快速的打造各种网络游戏。

(经常被问到承载上限, kbengine底层架构被设计为多进程分布式动态负载均衡方案， 理论上只需要不断扩展硬件就能够
不断增加承载上限, 单台机器的承载上限取决于游戏逻辑本身的复杂度。)



## KBEngine-cocos2dx 
   会实现KBEngine Cocos2dx 版本. 重点是客户端框架封装. 提供和KBE相似的开发流程. 可能无python脚本.但用CPP写逻辑也不错吧.
   
   地址在  
   
   https://github.com/cnsoft/kbengine-cocos2dx/tree/cocos2dx-cnsoft/kbe/src/client/cocos2dx 目录下.

   2014-07-01 PreAlpha is ready. Now, cocos2dx client can chat with unity3d client in the KBE same game server . by cnsoft
   
   2014-07-03 create submodule : kbengine_cocos2dx_demo. 
   
 ![screenshots1](https://raw.githubusercontent.com/cnsoft/kbengine-cocos2dx/cocos2dx-cnsoft/kbe/src/client/cocos2dx/snapshots/u_cocos2d_chat.PNG)
