使用方法:
1.请自行为你所使用的python安装与之匹配的django模块；
  1.1.如果不想安装django，有另一个选择:
  1.1.1.如果使用的是python2.6.6：可以选择进入到“kbe/tools/server/django_packages”目录，并在该目录下解压Django-1.6.11.tar.gz文件（解压到当前目录）
  1.1.2.如果使用的是python2.7或以上：可以选择进入到“kbe/tools/server/django_packages”目录，并在该目录下解压Django-1.8.9.tar.gz文件（解压到当前目录）
2.修改sync_db.bat、sync_db.sh以及run_server.bat、run_server.sh，使其正确指向你所使用的python的路径
3.第一次使用，需要初始化数据
  3.1.windows下python3.3 + django 1.8.9，运行“sync_db.bat”命令
  3.2.linux下python3.3 + django 1.8.9，运行“sync_db.sh”命令
  3.3.linux下python2.6 + django 1.6.11，运行“sync_db_dj-1.6.sh”命令
  3.4.windows下python2.6 + django 1.6.11，请参考“sync_db_dj-1.6.sh”自行创建.bat文件
4.运行服务器
  4.1.windows下，运行“run_server.bat”命令
  4.2.linux下，运行“run_server.sh”命令
5.在浏览器上输入“http://xxx.xxx.xxx.xxx:8000/wc/”进行访问，其中“xxx.xxx.xxx.xxx”为运行webconsole的机器IP

说明：
1.第一次使用此控制台时，默认的登录账号为“Admin”，默认密码为“123456”，此账号也是后台唯一的管理账号，登录后请及时修改密码；
2.第一次使用Admin进入后台，需要根据自己启动服务器的用户账号名和用户uid创建新的控制用户，创建完成后，需要退出Admin使用新用户登录才能进行实际的后台操作；
3.此后台为测试版，还有很多功能未完善，不建议使用在生产环境中。
4.此后台的开发环境为python3.3 + django 1.8.9，在linux下使用python2.6.6 + django-1.6.11测试通过。
5.此工具所有的操作都源于Machine，因此，想要使用这个工具的功能，必须确保machine进程正确运行。
6.有任何使用问题，请在KBEngine官方平台上提出。
