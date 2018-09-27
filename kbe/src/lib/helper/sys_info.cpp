// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "sys_info.h"

extern "C"
{
#include "sigar.h"
#include "sigar_format.h"
}

#ifndef CODE_INLINE
#include "sys_info.inl"
#endif

#if KBE_PLATFORM == PLATFORM_WIN32
#include <Iphlpapi.h>
#pragma comment (lib,"iphlpapi.lib") 
#else
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#endif

namespace KBEngine
{
KBE_SINGLETON_INIT(SystemInfo);

SystemInfo g_SystemInfo;

sigar_t *_g_sigarproclist = NULL;
sigar_proc_list_t _g_proclist;
sigar_t *_g_sigar_cpu = NULL;
sigar_cpu_t _g_old_cpu;

//-------------------------------------------------------------------------------------
bool hasPID(int pid, sigar_proc_list_t* proclist)
{
	for (size_t i = 0; i < proclist->number; i++)
	{
		sigar_pid_t cpid = proclist->data[i];
		if (cpid == pid)
		{
			sigar_proc_state_t procstate;
			if (sigar_proc_state_get(_g_sigarproclist, pid, &procstate) != SIGAR_OK)
			{
				return false;
			}

			return true;
		}
	}

	return false;
}

//-------------------------------------------------------------------------------------
SystemInfo::SystemInfo()
{
	totalmem_ = 0;

	// 不要在初始化中做这件事情，因为全局静态变量这里可能在main之前被调用一次
	//_autocreate();
	//getCPUPer();
	//getProcessInfo();
}

//-------------------------------------------------------------------------------------
SystemInfo::~SystemInfo()
{
	clear();
}

//-------------------------------------------------------------------------------------
void SystemInfo::clear()
{
	if(_g_sigarproclist)
	{
		sigar_proc_list_destroy(_g_sigarproclist, &_g_proclist);
		sigar_close(_g_sigarproclist);
		_g_sigarproclist = NULL;
	}

	if(_g_sigar_cpu)
	{
		sigar_close(_g_sigar_cpu);
		_g_sigar_cpu = NULL;
	}
}

//-------------------------------------------------------------------------------------
bool SystemInfo::_autocreate()
{
	if(_g_sigarproclist == NULL)
	{
		sigar_open(&_g_sigarproclist);

		int status = sigar_proc_list_get(_g_sigarproclist, &_g_proclist);
		if (status != SIGAR_OK) 
		{
			DEBUG_MSG(fmt::format("SystemInfo::autocreate: error: {} ({}) sigar_proc_list_get\n",
					   status, sigar_strerror(_g_sigarproclist, status)));

			sigar_close(_g_sigarproclist);
			_g_sigarproclist = NULL;
			return false;
		}
	}

	return true;
}

//-------------------------------------------------------------------------------------
bool SystemInfo::hasProcess(uint32 pid)
{
    if(!_autocreate())
		return false;

	return hasPID(pid, &_g_proclist);
}

//-------------------------------------------------------------------------------------
void SystemInfo::update()
{
    if(!_autocreate())
		return;
}

//-------------------------------------------------------------------------------------
uint32 SystemInfo::countCPU()
{
	static uint32 count = 0;

	if(count == 0)
	{
		int status = SIGAR_OK;
		sigar_t *sigarcpulist;
		sigar_cpu_list_t cpulist;
		sigar_open(&sigarcpulist);
		status = sigar_cpu_list_get(sigarcpulist, &cpulist);
		
		if (status != SIGAR_OK) 
		{
			DEBUG_MSG(fmt::format("error: {} ({}) cpu_list_get\n",
				   status, sigar_strerror(sigarcpulist, status)));

			count = 1;
		}
		else
		{
			count = cpulist.number;
			sigar_cpu_list_destroy(sigarcpulist, &cpulist);
		}

		sigar_close(sigarcpulist);
	}

	return count;
}

//-------------------------------------------------------------------------------------
SystemInfo::PROCESS_INFOS SystemInfo::getProcessInfo(uint32 pid)
{
	PROCESS_INFOS infos;
	infos.cpu = 0.f;
	infos.memused = 0;
	infos.error = false;
	
	bool tryed = false;

    if(!_autocreate())
		goto _END;

_TRYGET:
	if(!hasPID(pid, &_g_proclist))
	{
		//DEBUG_MSG(fmt::format("SystemInfo::getProcessInfo: error: not found pid({})\n", pid));
		
		if(!tryed)
		{
			clear();

			tryed = true;
			infos.error = true;

			//DEBUG_MSG(fmt::format("SystemInfo::getProcessInfo: try to find the pid({})\n", pid));

			if(_autocreate())
				goto _TRYGET;
		}

		goto _END;
	}
	else
	{
		if (tryed)
		{
			//DEBUG_MSG(fmt::format("SystemInfo::getProcessInfo: found pid({})\n", pid));
		}

		infos.error = false;
	}

	infos.cpu = getCPUPerByPID(pid);
	infos.memused = getMemUsedByPID(pid);

_END:
	return infos;
}

//-------------------------------------------------------------------------------------
float SystemInfo::getCPUPer()
{
	sigar_cpu_t current;
	 
	if(_g_sigar_cpu == NULL)
		sigar_open(&_g_sigar_cpu);

	// sigar_cpu_get(_g_sigar_cpu, &_g_old_cpu);
	 
	sigar_cpu_perc_t perc;

	// while(1)
	{
		sigar_cpu_get(_g_sigar_cpu, &current);
		sigar_cpu_perc_calculate(&_g_old_cpu, &current, &perc);
	 
		// std::cout << "CPU " << perc.combined * 100 << "%\n";
		_g_old_cpu = current;
	//	sleep(1000);
	}
	 
	 
	float ret = float(perc.combined) * 100.f;
	//sigar_close(sigar_cpu);
	// DEBUG_MSG(fmt::format("SystemInfo::getCPUPer(): {}\n", ret));
	return ret;
}

//-------------------------------------------------------------------------------------
float SystemInfo::getCPUPerByPID(uint32 pid)
{
	if(pid == 0)
	{
		pid = (uint32)getProcessPID();
	}

    float percent = 0.f;
	bool tryed = false;

_TRYGET:
	if(!hasPID(pid, &_g_proclist))
	{
		//DEBUG_MSG(fmt::format("SystemInfo::getCPUPerByPID: error: not found pid({})\n", pid));

		if(!tryed)
		{
			clear();
			tryed = true;

			if(_autocreate())
				goto _TRYGET;
		}

		return 0.f;
	}

	/*
	int status = SIGAR_OK;
	// for (size_t i = 0; i < proclist.number; i++)
    {
        sigar_proc_cpu_t cpu;
        status = sigar_proc_cpu_get(_g_sigarproclist, pid, &cpu);

		if (status != SIGAR_OK) 
		{
			DEBUG_MSG(fmt::format("error: {} ({}) proc_cpu_get({})\n",
				   status, sigar_strerror(_g_sigarproclist, status), pid));

			return 0.f;
		}
    }
	*/
  // sleep(1000);

	// for (size_t i = 0; i < proclist.number; i++)
    {
        sigar_proc_cpu_t cpu;
        int status = sigar_proc_cpu_get(_g_sigarproclist, pid, &cpu);
        if (status == SIGAR_OK)
        {
			/*
            sigar_proc_state_t procstate;
            status = sigar_proc_state_get(sigarproclist, pid, &procstate);

			if (status != SIGAR_OK) 
			{
				DEBUG_MSG(fmt::format("error: {} ({}) proc_state({})\n",
					   status, sigar_strerror(sigarproclist, status), pid));

				return 0.f;
			}
			*/
            percent = float(cpu.percent) * 100.f;

#if KBE_PLATFORM == PLATFORM_WIN32
			percent /= float(countCPU());
#endif

        }
		else
		{
			DEBUG_MSG(fmt::format("error: {} ({}) proc_cpu_get({})\n",
				   status, sigar_strerror(_g_sigarproclist, status), pid));

			return 0.f;
		}
    }

	return percent;
}

//-------------------------------------------------------------------------------------
SystemInfo::MEM_INFOS SystemInfo::getMemInfos()
{
	MEM_INFOS infos;

	sigar_t *sigar;
	sigar_open(&sigar);
	sigar_mem_t sigar_mem;
	sigar_mem_get(sigar, &sigar_mem);

	infos.total = sigar_mem.total;
	infos.free = sigar_mem.actual_free;
	infos.used = sigar_mem.actual_used;

	sigar_close(sigar);
	return infos;
}

//-------------------------------------------------------------------------------------
uint64 SystemInfo::totalmem()
{
	if(totalmem_ == 0)
		totalmem_ = getMemInfos().total;

	return totalmem_;
}

//-------------------------------------------------------------------------------------
uint64 SystemInfo::getMemUsedByPID(uint32 pid)
{
	if(pid == 0)
	{
		pid = (uint32)getProcessPID();
	}

	int status;
	sigar_uint64_t total = 0;
	bool tryed = false;

_TRYGET:
	if(!hasPID(pid, &_g_proclist))
	{
		//DEBUG_MSG(fmt::format("SystemInfo::getMemUsedByPID: error: not found pid({})\n", pid));

		if(!tryed)
		{
			clear();
			tryed = true;

			if(_autocreate())
				goto _TRYGET;
		}

		return 0;
	}

    //for (i=0; i<(int)proclist.number; i++) 
	{
        sigar_proc_state_t pstate;
        sigar_proc_time_t ptime;

        status = sigar_proc_state_get(_g_sigarproclist, pid, &pstate);
        if (status != SIGAR_OK) 
		{
			DEBUG_MSG(fmt::format("error: {} ({}) proc_state({})\n",
                   status, sigar_strerror(_g_sigarproclist, status), pid));
			
			goto _END;
        }

        status = sigar_proc_time_get(_g_sigarproclist, pid, &ptime);
        if (status != SIGAR_OK) 
		{
			DEBUG_MSG(fmt::format("error: {} ({}) proc_time({})\n",
                   status, sigar_strerror(_g_sigarproclist, status), pid));

           goto _END;
        }

		sigar_proc_mem_t proc_mem;
		status = sigar_proc_mem_get(_g_sigarproclist, pid, &proc_mem);

        if (status != SIGAR_OK) 
		{
			DEBUG_MSG(fmt::format("error: {} ({}) sigar_proc_mem_get({})\n",
                   status, sigar_strerror(_g_sigarproclist, status), pid));
            
			goto _END;
        }

		total = proc_mem.resident;
    }

_END:
	return total;
}

//-------------------------------------------------------------------------------------
std::vector< std::string > SystemInfo::getMacAddresses()
{
	std::vector< std::string > mac_addresses;

#if KBE_PLATFORM == PLATFORM_WIN32
	PIP_ADAPTER_INFO pIpAdapterInfo = new IP_ADAPTER_INFO();
	unsigned long size = sizeof(IP_ADAPTER_INFO);

	int ret_info = ::GetAdaptersInfo(pIpAdapterInfo, &size);

	if (ERROR_BUFFER_OVERFLOW == ret_info)
	{
		delete pIpAdapterInfo;
		pIpAdapterInfo = (PIP_ADAPTER_INFO)new unsigned char[size];
		ret_info = ::GetAdaptersInfo(pIpAdapterInfo, &size);
	}

	if (ERROR_SUCCESS == ret_info)
	{
		PIP_ADAPTER_INFO _pIpAdapterInfo = pIpAdapterInfo;
		while (_pIpAdapterInfo)
		{
			char MAC_BUF[256];
			std::string MAC;

			for (UINT i = 0; i < _pIpAdapterInfo->AddressLength; i++)
			{
				sprintf(MAC_BUF, "%02x", _pIpAdapterInfo->Address[i]);
				MAC += MAC_BUF;
			}

			std::transform(MAC.begin(), MAC.end(), MAC.begin(), tolower);
			mac_addresses.push_back(MAC);
			_pIpAdapterInfo = _pIpAdapterInfo->Next;
		}
	}

	if (pIpAdapterInfo)
	{
		delete pIpAdapterInfo;
	}

#else

	int fd;
	int interfaceNum = 0;
	struct ifreq buf[16];
	struct ifconf ifc;

	if ((fd = ::socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		::close(fd);
		return mac_addresses;
	}

	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = (caddr_t)buf;

	if (!ioctl(fd, SIOCGIFCONF, (char *)&ifc))
	{
		interfaceNum = ifc.ifc_len / sizeof(struct ifreq);
		while (interfaceNum-- > 0)
		{
			if (!ioctl(fd, SIOCGIFHWADDR, (char *)(&buf[interfaceNum])))
			{
				char MAC[19];
				memset(&MAC[0], 0, sizeof(MAC));

				sprintf(MAC, "%02x:%02x:%02x:%02x:%02x:%02x",
					(unsigned char)buf[interfaceNum].ifr_hwaddr.sa_data[0],
					(unsigned char)buf[interfaceNum].ifr_hwaddr.sa_data[1],
					(unsigned char)buf[interfaceNum].ifr_hwaddr.sa_data[2],
					(unsigned char)buf[interfaceNum].ifr_hwaddr.sa_data[3],
					(unsigned char)buf[interfaceNum].ifr_hwaddr.sa_data[4],
					(unsigned char)buf[interfaceNum].ifr_hwaddr.sa_data[5]);

				mac_addresses.push_back(MAC);
			}
			else
			{
				break;
			}
		}
	}

	::close(fd);

#endif

	return mac_addresses;
}


//-------------------------------------------------------------------------------------
} 

