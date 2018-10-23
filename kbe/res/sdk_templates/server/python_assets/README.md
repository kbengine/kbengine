这是一个KBEngine服务端资产库
========

##启动服务端

使用固定参数来启动：(参数的意义:http://www.kbengine.org/cn/docs/startup_shutdown.html)
	
	Linux:
		start_server.sh

	Windows:
		start_server.bat


##关闭服务端

快速杀死服务端进程:

	Linux:
		kill_server.sh

	Windows:
		kill_server.bat


如果是正式运营环境，应该使用安全的关闭方式，这种方式能够确保数据安全的存档，安全的告诉用户下线等等。

	Linux:
		safe_kill.sh

	Windows:
		safe_kill.bat