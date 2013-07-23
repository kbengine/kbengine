import commands
import time
import sys, os, time, signal, re

# yum -y install sysstat

warnCPUCheck = 70.0
warnIOCheck = 30.0

class DiskIO:
	def __init__(self, pname=None, pid=None, reads=0, writes=0):
		self.pname = pname
		self.pid = pid
		self.reads = 0
		self.writes = 0

def iotop():
	if os.getuid() != 0:
		print "must be run as root"
		sys.exit(0)

	os.system('echo 1 > /proc/sys/vm/block_dump')
	os.system("""echo "TASK              PID       READ      WRITE" >> kbmachine_watcher.log""")

	while True:
		os.system('dmesg -c > /tmp/diskio.log')
		l = []
		f = open('/tmp/diskio.log', 'r')
		line = f.readline()
		while line:
			m = re.match(\
				'^(\S+)\((\d+)\): (READ|WRITE) block (\d+) on (\S+)', line)
			if m != None:
				if not l:
					l.append(DiskIO(m.group(1), m.group(2)))
					line = f.readline()
					continue
				found = False
				for item in l:
					if item.pid == m.group(2):
						found = True
						if m.group(3) == "READ":
							item.reads = item.reads + 1
						elif m.group(3) == "WRITE":
							item.writes = item.writes + 1
				if not found:
					l.append(DiskIO(m.group(1), m.group(2)))
			line = f.readline()
		time.sleep(1)
		for item in l:
			s = "%-10s %10s %10d %10d" % \
				(item.pname, item.pid, item.reads, item.writes)

			os.system("""echo "%s" >> kbmachine_watcher.log""" % s)
		break
		
def signal_handler(signal, frame):
    os.system('echo 0 > /proc/sys/vm/block_dump')
    sys.exit(0)

def watcher0():
	while(1):
		warn = False
		
		procstats = commands.getstatusoutput("""ps -eo pid,comm,pcpu,pmem,cmd |sort -k3 -nr | head -15""")[1]
		for v in commands.getstatusoutput("""echo "%s" | awk '{print $3}'""" % procstats)[1].split("\n"):
			try:
				v = float(v)
			except:
				continue
			if v > warnCPUCheck:
				warn = True
				break
				
		iostates = commands.getstatusoutput("""iostat -d -x -k 1 1""")[1]
		for v in commands.getstatusoutput("""echo "%s" | awk '{print $12}'""" % iostates)[1].split("\n"):
			try:
				v = float(v)
			except:
				continue
			if v > warnIOCheck:
				warn = True
				break
				
		cpustates = commands.getstatusoutput("""mpstat -P ALL 1 1""")[1]
		if not warn:
			for v in commands.getstatusoutput("""echo "%s" | awk '{print $10}'""" % cpustates)[1].split("\n"):
				try:
					v = float(v)
				except:
					continue
				if (100 - v) > warnCPUCheck:
					warn = True
					break
		
		if warn:
			os.system("""echo =========================================================================================================== >> kbmachine_watcher.log""")
			os.system("""date >> kbmachine_watcher.log""")
			
			os.system("""echo " " >> kbmachine_watcher.log""")
			os.system("""echo "vmstat 1 1" >> kbmachine_watcher.log""")
			os.system("""vmstat 1 1 >> kbmachine_watcher.log""")
			
			os.system("""echo " " >> kbmachine_watcher.log""")
			os.system("""echo "uptime" >> kbmachine_watcher.log""")
			os.system("""uptime >> kbmachine_watcher.log""")
			
			os.system("""echo " " >> kbmachine_watcher.log""")
			os.system("""echo "iostat -d -x -k 1 1" >> kbmachine_watcher.log""")
			os.system("""echo "%s" >> kbmachine_watcher.log""" % iostates)
			
			os.system("""echo " " >> kbmachine_watcher.log""")
			os.system("""echo "mpstat -P ALL 1 1" >> kbmachine_watcher.log""")
			os.system("""echo "%s" >> kbmachine_watcher.log""" % cpustates)
			
			os.system("""echo " " >> kbmachine_watcher.log""")
			os.system("""echo "sar -n DEV 1 1" >> kbmachine_watcher.log""")
			os.system("""sar -n DEV 1 1 >> kbmachine_watcher.log""")
			
			os.system("""echo " " >> kbmachine_watcher.log""")
			os.system("""echo "ps -eo pid,comm,pcpu,pmem,cmd |sort -k3 -nr | head -15" >> kbmachine_watcher.log""")
			os.system("""echo "%s" >> kbmachine_watcher.log""" % procstats)

			os.system("""echo " " >> kbmachine_watcher.log""")
			os.system("""echo "iotop()" >> kbmachine_watcher.log""")
			iotop()
			
		time.sleep(0.01)

def watcher1():
	while True:
		warn = False
		cpustates = commands.getstatusoutput("""top -n 1""")[1]
		v = float(commands.getstatusoutput("""echo "%s" | grep 'Cpu(s)'""" % cpustates)[1].split(",")[3].split(" ")[1].split("%")[0])
		if v <= warnCPUCheck:
			warn = True

		if warn:
			os.system("""echo =========================================================================================================== >> kbmachine_watcher.log""")
			os.system("""date >> kbmachine_watcher.log""")
			
			os.system("""echo " " >> kbmachine_watcher.log""")
			os.system("""echo "%s" >> kbmachine_watcher.log""" % cpustates)
			
			os.system("""echo " " >> kbmachine_watcher.log""")
			os.system("""mpstat -P ALL 1 1  >> kbmachine_watcher.log""")
	
def watcher2():
	while True:
		warn = False
		
		procstats = commands.getstatusoutput("""ps -eo pid,comm,pcpu,pmem,cmd |sort -k3 -nr | head -15""")[1]
		for v in commands.getstatusoutput("""echo "%s" | awk '{print $3}'""" % procstats)[1].split("\n"):
			try:
				v = float(v)
			except:
				continue
			if v > warnCPUCheck:
				warn = True
				break

		if warn:
			os.system("""echo =========================================================================================================== >> kbmachine_watcher.log""")
			os.system("""date >> kbmachine_watcher.log""")
			
			os.system("""echo " " >> kbmachine_watcher.log""")
			os.system("""echo "ps -eo pid,comm,pcpu,pmem,cmd |sort -k3 -nr | head -15" >> kbmachine_watcher.log""")
			os.system("""echo "%s" >> kbmachine_watcher.log""" % procstats)
			
			os.system("""echo " " >> kbmachine_watcher.log""")
			os.system("""mpstat -P ALL 1 1  >> kbmachine_watcher.log""")
			
if __name__=="__main__":
	signal.signal(signal.SIGINT, signal_handler)
	os.system("""rm -r kbmachine_watcher.log""")
	
	argc = len(sys.argv)
	if argc == 1:
		watcher0()
	else:
		eval("watcher%s()" % (sys.argv[1]))