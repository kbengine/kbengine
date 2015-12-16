/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2016 KBEngine.

KBEngine is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

KBEngine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.
 
You should have received a copy of the GNU Lesser General Public License
along with KBEngine.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "sys_info.h"

extern "C"
{
#include "sigar.h"
#include "sigar_format.h"
}

#ifndef CODE_INLINE
#include "sys_info.inl"
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
		if(cpid == pid)
			return true;
	}

	return false;
}

//-------------------------------------------------------------------------------------
SystemInfo::SystemInfo()
{
	totalmem_ = 0;
	_autocreate();
	// getCPUPer();
	// getProcessInfo();
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
			DEBUG_MSG(fmt::format("SystemInfo::autocreate:error: {} ({}) sigar_proc_list_get\n",
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
	clear();

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
	clear();

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
		DEBUG_MSG(fmt::format("SystemInfo::getProcessInfo: error: not found pid({})\n", pid));
		
		if(!tryed)
		{
			clear();

			tryed = true;
			infos.error = true;

			if(_autocreate())
				goto _TRYGET;
		}

		goto _END;
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
		DEBUG_MSG(fmt::format("SystemInfo::getCPUPerByPID: error: not found pid({})\n", pid));

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
		DEBUG_MSG(fmt::format("SystemInfo::getMemUsedByPID: error: not found pid({})\n", pid));

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
} 

