使用方法:
1.为你所使用的python安装django模块；
2.修改sync_db.bat、sync_db.sh以及run_server.bat、run_server.sh，使其正确指向你所使用的python的路径
3.第一次使用请先运行sync_db.bat或sync_db.sh，以初始化数据
4.以后使用时使用run_server.bat或run_server.sh运行服务器
5.在浏览器上输入“http://xxx.xxx.xxx.xxx:8000/wc/”进行访问

说明：
1.第一次使用此控制台时，默认的登录账号为“Admin”，默认密码为“123456”，此账号也是后台唯一的管理账号，登录后请及时修改密码；
2.第一次使用Admin进入后台，需要根据自己启动服务器的用户账号名和用户uid创建新的控制用户，创建完成后，需要退出Admin使用新用户登录才能进行实际的后台操作；
3.此后台为测试版，还有很多功能未完善，不建议使用在生产环境中。
4.此后台的开发环境为python3.3 + django 1.8.9。
5.此工具所有的操作都源于Machine，因此，想要使用这个工具的功能，必须确保machine进程正确运行。
6.有任何使用问题，请在KBEngine官方平台上提出。
