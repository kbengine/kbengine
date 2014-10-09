#!/usr/bin/python  
# -*- coding:utf-8 -*-  

import urllib, socket
import tarfile, zipfile
import os, sys, re, platform, getopt, random, time, subprocess
from xml.etree import ElementTree as ET

if sys.hexversion >= 0x03000000:
	import urllib.request
	import configparser
else:
	import ConfigParser
	
# 源码及二进制发布网址
source_url = "https://github.com/kbengine/kbengine/releases/latest"
bin_zip_url = "https://sourceforge.net/projects/kbengine/files/bin/latest.zip/download"
bin_tgz_url = "https://sourceforge.net/projects/kbengine/files/bin/latest.tar.gz/download"
bin_mysql_url = "https://sourceforge.net/projects/kbengine/files/bin/deps/mysql-win32.msi/download"

# mysql 安装目录
mysql_root = ""
mysql_home = ""

# mysql版本信息
mysql_verinfo = ""

# mysql 端口
mysql_ip = ""
mysql_port = ""

# mysql root密码
mysql_root_password = ""

# mysql kbe账号名称和密码
mysql_kbe_name = ""
mysql_kbe_password = ""

# mysql db 名称
mysql_kbe_db_name = ""

# mysql服务名称
mysql_sercive_name = ""

# 根据root决定安装位置
KBE_ROOT = ''
KBE_RES_PATH = ''
KBE_HYBRID_PATH = ''
KBE_UID = ''
kbe_res_path = ""

# 工具环境变量名
INSTALLER_EVN_NAME = 'KBT'

_zip_kbengine_dirname = ""

def hello():
	# echoSystemEnvironment()
	# echoKBEEnvironment()

	OUT_MSG("###########################################################################")
	OUT_MSG("#   installer:                                                            #")
	OUT_MSG("#        KBEngine installation tool.                                      #")
	OUT_MSG("#        Install, Uninstall, Check the version,                           #")
	OUT_MSG("#        Environment settings, etc.                                       #")
	OUT_MSG("#   homepage:                                                             #")
	OUT_MSG("#         http://kbengine.org                                             #")
	OUT_MSG("#   sources:                                                              #")
	OUT_MSG("#         https://github.com/kbengine/kbengine/*/kbe/tools/server/install #")
	OUT_MSG("###########################################################################")
	OUT_MSG("")
    
def help():
	OUT_MSG("-------------------commands-----------------------")
	OUT_MSG("\tUsage:")
	OUT_MSG("\t\tpython installer.py [command]")
	OUT_MSG("")

	OUT_MSG("\t-i, --install:")
	OUT_MSG("\t\tInstall KBEngine.")
	OUT_MSG("\t\tinstaller.py -i : Install from local-disk(Source code), From the KBE_ROOT search.")
	OUT_MSG("\t\tinstaller.py --install=localsrc : Install from local-disk(Source code), From the KBE_ROOT search.")
	OUT_MSG("\t\tinstaller.py --install=remotesrc : Install from github(Source code).")
	OUT_MSG("\t\tinstaller.py --install=bin	: Install from Internet(Binary releases).")
	OUT_MSG("")
	
	OUT_MSG("\t-u, --uninstall:")
	OUT_MSG("\t\tUninstall KBEngine.")
	OUT_MSG("")
	
	OUT_MSG("\t-v, --version:")
	OUT_MSG("\t\tTo view the KBEngine version.")
	OUT_MSG("")
	
	OUT_MSG("\t-e, --evn:")
	OUT_MSG("\t\tThe output of the KBEngine environment.")
	OUT_MSG("")
	
	OUT_MSG("\t-r, --resetevn:")
	OUT_MSG("\t\tReset the KBEngine environment.")
	OUT_MSG("")
	
	OUT_MSG("\t-h, --help:")
	OUT_MSG("\t\tList all of the command descriptions.")
	OUT_MSG("--------------------------------------------------")

def OUT_MSG(msg):
	print(msg)
    
def INFO_MSG(msg):
	print(msg)

def ERROR_MSG(msg):
	print('[ERROR] ' + msg)

def WARING_MSG(msg):
	print('[WARING] ' + msg)

def getInput(s):
	if sys.hexversion >= 0x03000000:
		return input(s)
	
	return raw_input(s)
	
def echoKBEEnvironment():
	global KBE_ROOT
	global KBE_RES_PATH
	global KBE_HYBRID_PATH
	global KBE_UID
	
	KBE_ROOT = getEnvironment('user', 'KBE_ROOT')
	KBE_RES_PATH = getEnvironment('user', 'KBE_RES_PATH')
	KBE_HYBRID_PATH = getEnvironment('user', 'KBE_HYBRID_PATH')
	_checkKBEEnvironment(False)
	
	OUT_MSG("KBE_ROOT=" + KBE_ROOT)
	OUT_MSG("KBE_RES_PATH=" + KBE_RES_PATH)
	OUT_MSG("KBE_HYBRID_PATH=" + KBE_HYBRID_PATH)
	OUT_MSG("kbe_core_res_path = %s" % kbe_res_path)

def findKBEngine(dir):
	if len(_zip_kbengine_dirname) > 0:
		return dir + "/" + _zip_kbengine_dirname + "/"
		
	if dir[-1] != '/' and dir[-1] != '\\':
		dir += '/'
	
	paths = []
	for x in os.listdir(dir):
		if "kbengine" in x:
			if os.path.isfile(dir + x + "/kbe/res/server/kbengine_defs.xml"):
				paths.append(dir + x + "/")
	
	return paths
	
def resetKBEEnvironment():
	global KBE_ROOT
	global KBE_RES_PATH
	global KBE_HYBRID_PATH
	global KBE_UID
	
	KBE_ROOT = getEnvironment('user', 'KBE_ROOT')
	KBE_RES_PATH = getEnvironment('user', 'KBE_RES_PATH')
	KBE_HYBRID_PATH = getEnvironment('user', 'KBE_HYBRID_PATH')
	KBE_UID = getEnvironment('user', 'UID')
	
	# 如果没有找到root环境配置， 则尝试从当前目录识别是否在kbengine目录中， 如果在
	# 则我们自动设置环境
	x_KBE_ROOT = KBE_ROOT
	x_KBE_RES_PATH = KBE_RES_PATH
	x_KBE_HYBRID_PATH = KBE_HYBRID_PATH
	x_KBE_UID = KBE_UID
	
	if len(KBE_ROOT) == 0:
		curr = os.getcwd()
		curr = curr.replace("\\", "/")
		
		if "kbe/tools/server/install" in curr:
			curr = curr.replace("kbe/tools/server/install", "").replace("//", "/")
			x_KBE_ROOT = curr

			if x_KBE_ROOT[-1] != "/":
				x_KBE_ROOT += "/"
		else:
			x_KBE_ROOT = findKBEngine(os.getcwd())
			x_KBE_ROOT.replace("\\", "/").replace("//", "/")
			
		x_KBE_RES_PATH = x_KBE_ROOT + "kbe/res;" + x_KBE_ROOT + "demo/;" + x_KBE_ROOT + "demo/res"
			
	if platform.architecture()[0] == '32bit':
		x_KBE_HYBRID_PATH = x_KBE_ROOT + "kbe/bin/Hybrid"
	else:
		x_KBE_HYBRID_PATH = x_KBE_ROOT + "kbe/bin/Hybrid64"
		if not os.path.isdir(x_KBE_HYBRID_PATH):
			x_KBE_HYBRID_PATH = x_KBE_ROOT + "kbe/bin/Hybrid"
			
	if len(KBE_UID) == 0:
		x_KBE_UID = str(random.randint(1, 65535))

	while True:
		INFO_MSG("\nKBE_ROOT current: %s" % (KBE_ROOT))
		KBE_ROOT = getInput('reset KBE_ROOT(No input is [%s]):' % (x_KBE_ROOT)).strip()
		if len(KBE_ROOT) == 0:
			if len(x_KBE_ROOT) == 0:
				INFO_MSG('KBE_ROOT: no change!')
			else:
				KBE_ROOT = x_KBE_ROOT
				
		INFO_MSG("\nKBE_RES_PATH current: %s" % (x_KBE_RES_PATH))
		KBE_RES_PATH = getInput('reset KBE_RES_PATH(No input is [%s]):' % (x_KBE_RES_PATH)).strip()
		if len(KBE_RES_PATH) == 0:
			if len(x_KBE_RES_PATH) == 0:
				INFO_MSG('KBE_RES_PATH: no change!')
			else:
				KBE_RES_PATH = x_KBE_RES_PATH
		
		INFO_MSG("\nKBE_HYBRID_PATH current: %s" % (x_KBE_HYBRID_PATH))
		KBE_HYBRID_PATH = getInput('reset KBE_HYBRID_PATH(No input is [%s]):' % (x_KBE_HYBRID_PATH)).strip()
		if len(KBE_HYBRID_PATH) == 0:
			if len(x_KBE_HYBRID_PATH) == 0:
				INFO_MSG('KBE_HYBRID_PATH: no change!')
			else:
				KBE_HYBRID_PATH = x_KBE_HYBRID_PATH

		INFO_MSG("\nKBE_UID current: %s" % (x_KBE_UID))
		if platform.system() == 'Windows':
			KBE_UID = getInput('reset KBE_UID(No input is [%s]):' % (x_KBE_UID)).strip()
		else:
			# Linux需要修改系统用户ID
			username = getInput('os system-username(kbe):').strip()
			KBE_UID = getInput('usermod -u [curruid=%s] %s, Enter new uid:' % (KBE_UID, username)).strip()
			
		if len(KBE_UID) == 0:
			if len(x_KBE_UID) == 0:
				INFO_MSG('KBE_UID: no change!')
			else:
				KBE_UID = x_KBE_UID

		if len(KBE_ROOT) > 0:
			setEnvironment('user', 'KBE_ROOT', KBE_ROOT)
			
		if len(KBE_RES_PATH) > 0:
			setEnvironment('user', 'KBE_RES_PATH', KBE_RES_PATH)

		if len(KBE_HYBRID_PATH) > 0:
			setEnvironment('user', 'KBE_HYBRID_PATH', KBE_HYBRID_PATH)

		if len(KBE_UID) > 0:
			setEnvironment('user', 'UID', KBE_UID)
		
		if _checkKBEEnvironment(True):
			break
		
		INFO_MSG("\n---------------------------------------------")
		
	echoKBEEnvironment()

def get_linux_ugid(username):  
	fileobj1 = open('/etc/passwd')  
	fileobj2 = open('/etc/group')  

	uid = None
	gid = None

	for line in fileobj1:  
		if line.startswith(username + ':'):  
			tmp = line.split(':')  
			uid = tmp[2]  

	for line in fileobj2:  
		if line.startswith(username + ':'):  
			tmp = line.split(':')  
			gid = tmp[2]  

	return (uid, gid)  
    
def _checkKBEEnvironment(is_get_error):
	global KBE_ROOT
	global KBE_RES_PATH
	global KBE_HYBRID_PATH
	global kbe_res_path
	
	KBE_ROOT = getEnvironment('user', 'KBE_ROOT')
	KBE_RES_PATH = getEnvironment('user', 'KBE_RES_PATH')
	KBE_HYBRID_PATH = getEnvironment('user', 'KBE_HYBRID_PATH')
	
	kbe_path = KBE_ROOT + "/kbe"
	kbe_path = kbe_path.replace("\\", "/").replace("//", "/")
	if not os.path.isdir(kbe_path):
		if is_get_error:
			ERROR_MSG("KBE_ROOT: is error! The directory or file not found:\n%s" % (kbe_path)) 
		return False
	
	paths = []

	checkKBERes = [
		"server/kbengine_defs.xml",
		"scripts",
	]
	
	checkKBEUserRes = [
		"server/kbengine.xml",
		"scripts/entities.xml",
	]
	
	KBE_RES_PATH = KBE_RES_PATH.replace("%KBE_ROOT%", KBE_ROOT)
	KBE_RES_PATH = KBE_RES_PATH.replace("$KBE_ROOT", KBE_ROOT)
	
	if ";" in KBE_RES_PATH:
		paths = KBE_RES_PATH.split(";")
	else:
		paths = KBE_RES_PATH.split(":")
	
	for path in paths:
		if not os.path.isdir(path):
			if is_get_error:
				ERROR_MSG("KBE_RES_PATH: is error! The directory or file not found:\n%s" % (path)) 
			return False
	
	if not os.path.isdir(KBE_HYBRID_PATH):
		if is_get_error:
			WARING_MSG("KBE_HYBRID_PATH: is error! The directory or file not found:\n%s" % (KBE_HYBRID_PATH)) 
	
	kbe_res_path = ""
	for res in checkKBERes:
		found = False
		
		tmp = ""
		for path in paths:
			if path[-1] != '/' and path[-1] != '\\':
				path += '/'
				
			path1 = path + res
			tmp += path1 + "\n"
			if os.path.isdir(path1) or os.path.isfile(path1):
				kbe_res_path = path
				found = True
				break
		
		if not found:
			if is_get_error:
				ERROR_MSG("KBE_RES_PATH: is error! The directory or file not found:\n%s" % (tmp)) 
			return False
	
	for res in checkKBEUserRes:
		found = False
		
		tmp = ""
		for path in paths:
			
			if path[-1] != '/' and path[-1] != '\\':
				path += '/'
				
			path = path + res
			tmp += path + "\n"
			
			if os.path.isdir(path) or os.path.isfile(path):
				found = True
				break
		
		if not found:
			if is_get_error:
				ERROR_MSG("KBE_RES_PATH: is error! The directory or file not found:\n%" % (tmp)) 
			return False
	
	return True
    
def echoSystemEnvironment():
	OUT_MSG("platform=" + platform.platform())
	OUT_MSG("python_version=" + sys.version)
	OUT_MSG("python_path=" + sys.executable)
	OUT_MSG("currpath=" + os.getcwd())
	
def echoKBEVersion():
	INFO_MSG("version=0.0.1")
	
	if getInput("View the latest version of GitHub? [yes|no]") != "yes":
		return
		
	OUT_MSG("")
	INFO_MSG("Check out the latest version...")
	urls = get_sources_infos()
	src_master_zip_url = urls[0]
	src_zip_url = urls[1]
	src_tgz_url = urls[2]
	release_title = urls[3]
	descrs = urls[4]
	INFO_MSG("-------------------------")
	INFO_MSG(release_title)
	INFO_MSG(descrs)
    
def setEnvironment(scope, name, value):
	assert scope in ('user', 'system')
	#INFO_MSG('set environment: name=%s, value=%s' % (name, value))

	if platform.system() == 'Windows':
		try:
			import winreg
		except:
			import _winreg as winreg

		root = winreg.HKEY_CURRENT_USER
		subkey = 'Environment'

		if scope != 'user':
			root = winreg.HKEY_LOCAL_MACHINE
			subkey = r'SYSTEM\CurrentControlSet\Control\Session Manager\Environment'

		# Note: for 'system' scope, you must run this as Administrator
		key = winreg.OpenKey(root, subkey, 0, winreg.KEY_ALL_ACCESS)
		winreg.SetValueEx(key, name, 0, winreg.REG_EXPAND_SZ, value)
		winreg.CloseKey(key)
	else:
		if name.lower() == 'uid':
			return

def getEnvironment(scope, name):
    assert scope in ('user', 'system')
    
    if platform.system() == 'Windows':
        from subprocess import check_call
        try:
            import winreg
        except:
            import _winreg as winreg

        root = winreg.HKEY_CURRENT_USER
        subkey = 'Environment'
            
        if scope != 'user':
            root = winreg.HKEY_LOCAL_MACHINE
            subkey = r'SYSTEM\CurrentControlSet\Control\Session Manager\Environment'

        key = winreg.OpenKey(root, subkey, 0, winreg.KEY_READ)
        try:
            value, _ = winreg.QueryValueEx(key, name)
        except WindowsError:
            value = ''
        return value

def getMysqlConfig():
	global mysql_root
	cfg = "my.ini"

	if platform.system() != 'Windows':
		cfg = "my.cnf"

	cnf = mysql_root + cfg

	while True:
		if not os.path.isfile(cnf):
			if not os.path.isfile('/etc/' + cfg):
				if not os.path.isfile(mysql_root + "my-default.ini"):
					ERROR_MSG("not found mysqlconfig[%s]." % cnf)
					
					if platform.system() == 'Windows':
						cnf = getInput("Enter the mysqlconfig path(such as [c:/mysql/my.ini or my-default.ini]):")
					else:
						cnf = getInput("Enter the mysqlconfig path(such as [/etc/my.cnf]):")
				else:
					cnf = mysql_root + "my-default.ini"
			else:
				cnf = '/etc/' + cfg
		else:
			break

	config = configparser.ConfigParser()
	config.read(cnf)
	return config, cnf

def installMysql():    
    file = 'mysql-win32.msi'
    
    try:
        os.remove(file)
    except:
        pass
    
    file = download(bin_mysql_url, file)[0]
    INFO_MSG("wait for install:" + file)
    syscommand(file, False)
    os.remove(file)

def restartMsql():
	global mysql_sercive_name
	INFO_MSG('Try to stop %s...' % mysql_sercive_name)
	syscommand('net stop ' + mysql_sercive_name, False)
	assert (not findMysqlService()) and 'Unable to stop the MySQL, You need administrator privileges.'
	
	INFO_MSG('Try to start %s...' % mysql_sercive_name)
	ret, cret = syscommand('net start ' + mysql_sercive_name, False)
	assert findMysqlService() and 'Unable to start the MySQL, You need administrator privileges.'
	INFO_MSG('MySQL is ok')

def findMysqlService():
	global mysql_sercive_name
	ret, cret = syscommand('net start', True)
	
	for s in ret:
		if "mysql" in s.strip().lower():
			if len(mysql_sercive_name) == 0:
				mysql_sercive_name = s.strip()
				INFO_MSG("found mysql service[%s]" % mysql_sercive_name)
			return True

	return False

def syscommand(cmdstr, isGetRet):
	filename = os.getcwd() + "/" + str(random.randint(0, 9999999)) + ".log"
	
	if isGetRet:
		cmdstr = "(" + cmdstr + ")"
		cmdstr += " > \"" + filename + "\""
		
	cret = subprocess.Popen(cmdstr, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT).stdout.readlines()
	#if len(ret) > 0:
	#	os.remove(filename)
	#	return ret
	
	ret = []
	
	if not isGetRet:
		return (ret, [],)

	i = 0
	while True:
		if os.path.isfile(filename):
			break
			
		INFO_MSG("wating(%d) ..." % i)
		i += 1
		
		INFO_MSG(cmdstr)
		
		time.sleep(0.1)
		
	f = open(filename)
	
	while f:
		line = f.readline()
		if not line:
			break
		
		ret.append(line)

	f.close()
	os.remove(filename)

	return (ret, cret,)

def modifyKBEConfig():
	_checkKBEEnvironment(False)
	
	global mysql_ip
	global mysql_port
	global mysql_kbe_name
	global mysql_kbe_password
	global mysql_kbe_db_name
	global kbe_res_path
	
	kbengine_defs = kbe_res_path + "server/kbengine_defs.xml"

	if len(mysql_ip) == 0:
		mysql_ip = "localhost"
	
	if len(mysql_port) == 0:
		mysql_port = "0"
	
	if len(mysql_kbe_name) == 0:
		mysql_kbe_name = "kbe"

	if len(mysql_kbe_password) == 0:
		mysql_kbe_password = "kbe"
		
	if len(mysql_kbe_db_name) == 0:
		mysql_kbe_db_name = "kbe"
		
	state = 0

	f = open(kbengine_defs)
	newxml = []
	for x in f.readlines():
		if "</dbmgr>" in x:
			state = -1

		if state == 0:
			if "<dbmgr>" in x:
				state += 1
				
		if state == 1:
			if "<host>" in x and "localhost" in x:
				x = x.replace("localhost", mysql_ip)

			if "<port>" in x and "0" in x:
				x = x.replace("0", mysql_port)

			if "<username>" in x and "kbe" in x:
				x = x.replace("kbe", mysql_kbe_name)

			if "<password>" in x and "kbe" in x:
				x = x.replace("kbe", mysql_kbe_password)

			if "<databaseName>" in x and "kbe" in x:
				x = x.replace("kbe", mysql_kbe_db_name)
	                
		newxml.append(x)

	f.close()
	f = open(kbengine_defs, "w")
	f.writelines(newxml)
	f.close()
	return True
    
def createDatabase():
	global mysql_home
	mysql_home = getEnvironment('user', 'MYSQL_HOME')
	mysql_home = mysql_home.replace("\\", "/")
	
	global mysql_root
	global mysql_verinfo
	global mysql_ip
	global mysql_port
	global mysql_root_password
	global mysql_kbe_name
	global mysql_kbe_password
	global mysql_kbe_db_name
	
	lower_case_table_names = ''
    
	def getRootOpt(rootpasswd):
		if len(rootpasswd) == 0:
			return "-uroot "
		return "-uroot -p" + rootpasswd
    
	rootusePortArgs = ""
	mysql_ip = "localhost"
	mysql_port = "3306"
    
	while True:
		if len(mysql_home) > 0:
			if mysql_home[-1] != '\\' and mysql_home[-1] != '/':
				mysql_home += "/"
		
		ret, cret = syscommand("\"" + mysql_home + "mysql\" --help", True)
		if len(ret) == 0:
			if platform.system() == 'Windows':
				mysql_home = getInput("\Enter mysql.exe path(such as: C:\\MySQL Server 5.1\\bin\\):")
			else:
				tmp = ""
				ret = subprocess.Popen("whereis mysql", shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT).stdout.readlines()[0].split()
				if len(ret) > 0:
					tmp = ret[1].replace("mysql", "")
				
				if len(tmp) == 0:
					mysql_home = getInput("\Enter mysql(The executable file) path(such as: /usr/bin/):")
				
			continue
		else:
			setEnvironment('user', 'MYSQL_HOME', mysql_home)
            
		if len(mysql_root_password) == 0:
			if len(mysql_root_password) == 0:
				mysql_root_password = getInput("- Enter mysql-root password:")
				
			sql = "\"select VERSION();show variables like \'port\';show variables like \'lower_case_table_names\';select @@basedir as basePath from dual\""
			cmd = "\"" + mysql_home + ("mysql\" %s%s -hlocalhost -e" % (getRootOpt(mysql_root_password), rootusePortArgs)) + sql
			ret, cret = syscommand(cmd, True)
 
			if len(ret) == 0:
				mysql_root_password = ""
				ERROR_MSG("The password or port error! \n\n\terrorinfos - %s\n\n\tcommand - %s\n\n" % (cret, cmd))
				if str(cret).find("2003") > 0 and str(cret).find("10061") > 0:
					mysql_port = getInput("Please enter the MySQL port number:")
					rootusePortArgs = " -P" + mysql_port

				continue
			else:
				mysql_verinfo = ret[1].strip()
				INFO_MSG("MySQL_Version:" + mysql_verinfo)

				mysql_port = ret[3].replace('port', '').strip()
				INFO_MSG("MySQL_Port:" + mysql_port)

				lower_case_table_names = ret[5].replace('lower_case_table_names', '').strip()

				mysql_root = ret[7].strip()

				if lower_case_table_names != '0':
					ERROR_MSG('mysql lower_case_table_names not is 0')
					config, cnf = getMysqlConfig()
					INFO_MSG('Attempt to modify the [%s]...' % cnf)
					config.set('mysqld', 'lower_case_table_names', '0')
					config.write(open(cnf, "w"))
					restartMsql()
					continue
                
				sql = "\"delete from user where user=\'\';FLUSH PRIVILEGES\""
				cmd = "\"" + mysql_home + ("mysql\" %s%s -hlocalhost -e" % (getRootOpt(mysql_root_password), rootusePortArgs)) + sql + " mysql"
				syscommand(cmd, False)

		if len(mysql_kbe_name) == 0:
			OUT_MSG('')
			INFO_MSG("create kbe mysql-account:")
			mysql_kbe_name = getInput("- username(Do not enter the default is \'kbe\')): ")
			if len(mysql_kbe_name) == 0:
				mysql_kbe_name = "kbe"
                        
			mysql_kbe_password = getInput("- password(Do not enter the default is \'kbe\')): ")
			if len(mysql_kbe_password) == 0:
				mysql_kbe_password = "kbe"

			INFO_MSG('Create kbe-account: name=%s, password=%s successfully!' % (mysql_kbe_name, mysql_kbe_password))
            
			if len(mysql_kbe_db_name) == 0:
				OUT_MSG('')
				mysql_kbe_db_name = getInput("Create game database(Do not enter the default is \'kbe\'):")
				if len(mysql_kbe_db_name) == 0:
					mysql_kbe_db_name = "kbe"
                    
			sql = "\"grant all privileges on *.* to %s@\'%%\' identified by \'%s\';grant select,insert,update,delete,create,drop on *.* to %s@\'%%\' identified by \'%s\';FLUSH PRIVILEGES\"" % (mysql_kbe_name, mysql_kbe_password, mysql_kbe_name, mysql_kbe_password)
			cmd = "\"" + mysql_home + ("mysql\" %s%s -hlocalhost -e" % (getRootOpt(mysql_root_password), rootusePortArgs)) + sql + " mysql"
			syscommand(cmd, False)

			sql = "\"delete from user where user=\'\';FLUSH PRIVILEGES\""
			cmd = "\"" + mysql_home + ("mysql\" %s%s -hlocalhost -e" % (getRootOpt(mysql_root_password), rootusePortArgs)) + sql + " mysql"
			syscommand(cmd, False)
                
		# 如果表存在则报错
		has_db_sql = "\"SELECT * FROM information_schema.SCHEMATA where SCHEMA_NAME=\'%s\'\""  % (mysql_kbe_db_name)
		cmd = "\"" + mysql_home + ("mysql\" -u%s -p%s -hlocalhost -P%s -e" % (mysql_kbe_name, mysql_kbe_password, mysql_port)) + has_db_sql
		ret, cret = syscommand(cmd, True)

		if len(ret) > 0:
			ERROR_MSG("database[%s] has exist!" % (mysql_kbe_db_name))
			mysql_kbe_db_name = "";
			mysql_kbe_db_name = "";
			mysql_kbe_name = ''
			continue

		# 创建表
		sql = "\"create database %s\"" % (mysql_kbe_db_name)
		cmd = "\"" + mysql_home + ("mysql\" -u%s -p%s -hlocalhost -P%s -e" % (mysql_kbe_name, mysql_kbe_password, mysql_port)) + sql
		syscommand(cmd, False)

		# 再次检查是否创建成功， 成功返回值>0否则重新请求创建
		cmd = "\"" + mysql_home + ("mysql\" -u%s -p%s -hlocalhost -P%s -e" % (mysql_kbe_name, mysql_kbe_password, mysql_port)) + has_db_sql
		ret, cret = syscommand(cmd, True)
		if len(ret) == 0:
			ERROR_MSG("database is error! %s" % (cret))
			mysql_kbe_db_name = "";
			mysql_kbe_db_name = "";
			mysql_kbe_name = ''
			continue
		else:
			INFO_MSG("create database(%s) is successfully!" % mysql_kbe_db_name)
			break

		return True

def checkMysql():
	global mysql_ip
	global mysql_port
	global mysql_kbe_name
	global mysql_kbe_password
	global mysql_kbe_db_name
	
	ret = getInput("- MySQL is installed on the remote machine?[yes/no]")
	if ret == 'yes':
		while True:
			if len(mysql_ip) == 0:
				mysql_ip = getInput("- Enter mysql ip-address:")
				continue
				
			if len(mysql_port) == 0:
				mysql_port = getInput("- Enter mysql ip-port:")
				continue
				
			if len(mysql_kbe_name) == 0:
				mysql_kbe_name = getInput("- Enter mysql-account:")
				continue
				
			if len(mysql_kbe_password) == 0:
				mysql_kbe_password = getInput("- Enter mysql-password:")
				continue
				
			if len(mysql_kbe_db_name) == 0:
				mysql_kbe_db_name = getInput("- Enter mysql-databaseName:")
				continue
			
			break
			
		return True
				
	itry = 0
	
	syscommand('net start mysql', False)
	found = findMysqlService()

	INFO_MSG("MySQL is installed on the local.")
	INFO_MSG("- check mysql service...")
	restartMsql()

	while True:
		if not found:
			found = findMysqlService()
        
		if not found:
			INFO_MSG("")
			ERROR_MSG("-  not found MySQL")
			if itry == 1:
				return False

			ret = getInput("-  Allow automatic installation of MySQL? [yes/no]")
			if ret != 'yes':
				return False
			else:
				if not installMysql():
					return False
				else:
					itry += 1
		else:
			break

	createDatabase()
	return found

def checkGit():
	pass

def checkKBEEnvironment():
	if not _checkKBEEnvironment(True):
		resetKBEEnvironment()
		
	return True
	
def checkDeps():
	setEnvironment('user', INSTALLER_EVN_NAME, os.getcwd())
    
	deps = {
			"kbe_environment": checkKBEEnvironment,
	        "mysql" : checkMysql,
	        # "git" : checkGit
	    }

	OUT_MSG("")
	INFO_MSG("Check the dependences:")
	for dep in deps:
		INFO_MSG("- %s: checking..." % dep)
		ret = deps[dep]()
		if ret:
			INFO_MSG("- %s: yes" % dep)
		else:
			ERROR_MSG("- %s: no" % dep)
			assert False
	
	return True
	
def get_sources_infos():
	try:
		response = urllib.request.urlopen(source_url)
	except:
		response = urllib.urlopen(source_url)
        
	html = response.read().decode("utf8")
	ziplist = re.compile("""=\"[a-zA-Z0-9//\/\.?]+.zip""").findall(html)
	tgzlist = re.compile("""=\"[a-zA-Z0-9//\/\.?]+.gz""").findall(html)

	src_master_zip_url = ziplist[0].replace("=\"", "https://github.com")
	src_zip_url = ziplist[1].replace("=\"", "https://github.com")
	src_tgz_url = tgzlist[0].replace("=\"", "https://github.com")

	# title
	tag_start = """<h1 class="release-title">"""
	tag_end = """</h1>"""

	release_title = html
	release_title = release_title[release_title.find(tag_start) + len(tag_start):]
	release_title = release_title[:release_title.find(tag_end)]
	release_title = re.compile("""\<a(?:\\s+.+?)*?\\s+href=\"(.*?\"\>)(.*?)\<\/a\>""").findall(release_title)
	release_title = release_title[0][1]

	# descriptions
	tag_start = """<ul class="task-list">"""
	tag_end = """</ul>"""

	descrs = html
	descrs = descrs[descrs.find(tag_start) + len(tag_start):]
	descrs = descrs[:descrs.find(tag_end)]
	descrs = descrs.replace("\n", "")
	descrs = descrs.replace("<li>", "\t- ")
	descrs = descrs.replace("</li>", "\n")

	# downloads
	#print("\ndownloads:")
	#print("found:" + src_zip_url)
	#print("found:" + src_tgz_url)
	return (src_master_zip_url, src_zip_url, src_tgz_url, release_title, descrs)
    
def download_hookreport(count, block_size, total_size):
    INFO_MSG('%d/%d (%02d%%)' % (count * block_size, total_size, 100.0 * count * block_size / total_size))

def download(currurl, fname = None):
    OUT_MSG("")
    INFO_MSG("Downloading from " + currurl)
    
    try:
        return urllib.urlretrieve(currurl, filename = fname, reporthook = download_hookreport)
    except:
        pass
    
    return urllib.request.urlretrieve(currurl, filename = fname, reporthook = download_hookreport)

def download_sources(release = True):
	global _zip_kbengine_dirname
	_zip_kbengine_dirname = ""
	
	OUT_MSG("")
	INFO_MSG("Getting the latest source code...")
	urls = get_sources_infos()
	src_master_zip_url = urls[0]
	src_zip_url = urls[1]
	src_tgz_url = urls[2]
	release_title = urls[3]
	descrs = urls[4]

	currurl = src_zip_url

	# 如果release为False则下载git master版本
	if not release:
		currurl = src_master_zip_url

	INFO_MSG("")
	INFO_MSG(release_title)
	INFO_MSG(descrs)

	file = download(currurl)[0]
	namelist = extract_file(file)
	os.remove(file)

	for n in namelist[0].replace("\\", "/").split("/"):
		if "kbengine" in n:
			_zip_kbengine_dirname = n
			break
	
def download_binary():
	global _zip_kbengine_dirname
	_zip_kbengine_dirname = ""

	OUT_MSG("")
	INFO_MSG("Getting the latest KBEngine...")        
	file = download(bin_zip_url)[0]
	namelist = extract_file(file)
	os.remove(file)

	for n in namelist[0].replace("\\", "/").split("/"):
		if "kbengine" in n:
			_zip_kbengine_dirname = n
			break
	
def extract_file(src_file, extractPath = "./"):
	OUT_MSG("")
	INFO_MSG("unzip(%s)..." % (src_file))

	f = zipfile.ZipFile(src_file, 'r')
	total_count = len(f.namelist())
	count = 0
	
	namelist = f.namelist()
	for file in f.namelist():
		f.extract(file, extractPath)
		count += 1
		INFO_MSG("extract: %s %d/%d(%d%%)" % (file, count, total_count, (count / total_count) * 100))

	OUT_MSG("")
	INFO_MSG("unzip(%s) is completed(%d)!" % (src_file, len(namelist)))
	return namelist
	
def sourceinstall(islocal):
	if not islocal:
		download_sources()
		
	checkDeps()
	modifyKBEConfig()
	INFO_MSG("KBEngine has been successfully installed.")
	
def binaryinstall():
	download_binary()
	checkDeps()
	modifyKBEConfig()
	INFO_MSG("KBEngine has been successfully installed.")
	
def uninstall():
	pass
	
def processCommand():
	shortargs = 'vheuir'  
	longargs = ['install=', 'uninstall', 'version', 'evn', 'resetevn', 're', 'help']  

	try:
		opts, args = getopt.getopt(sys.argv[1:], shortargs, longargs) 
	except getopt.GetoptError: 
		ERROR_MSG('argv is error! argv=%s' % (sys.argv[1:]))
		hello()
		help() 
		return

	if len(opts) == 0:
		hello()
		help() 
		return

	for o, a in opts: 
		if o in ("-v", "--version"):  
			echoKBEVersion()
			sys.exit() 
		elif o in ("-h", "--help"):  
			hello()
			help()  
			sys.exit()  
		elif o in ("-e", "--evn"):  
			echoKBEEnvironment()
			sys.exit() 
		elif o in ("-r", "--resetevn"):  
			resetKBEEnvironment()
			sys.exit() 
		elif o in ("-u", '--uninstall'):  
			hello()
			uninstall()
			sys.exit() 
		elif o in ("-i", "--install"):  
			hello()
			
			if a == "remotesrc":
				sourceinstall(False)
			elif a == "bin":
				binaryinstall()
			else:
				sourceinstall(True)
				
			sys.exit() 
		else:  
			assert False, "unhandled option"  
 
if __name__ == "__main__":
	processCommand()
    

    
