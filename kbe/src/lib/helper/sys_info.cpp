/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 KBEngine.

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

#include "sys_info.hpp"

extern "C"
{
#include "sigar.h"
#include "sigar_format.h"
}

#ifndef CODE_INLINE
#include "sys_info.ipp"
#endif


namespace KBEngine
{
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
float SystemInfo::getCPUPer()
{
	sigar_t *sigar_cpu;
	sigar_cpu_t old;
	sigar_cpu_t current;
	 
	sigar_open(&sigar_cpu);
	sigar_cpu_get(sigar_cpu, &old);
	 
	sigar_cpu_perc_t perc;

	// while(1)
	{
		sigar_cpu_get(sigar_cpu, &current);
		sigar_cpu_perc_calculate(&old, &current, &perc);
	 
		// std::cout << "CPU " << perc.combined * 100 << "%\n";
		old = current;
		Sleep(1000);
	}
	 
	 
	 float ret = float(perc.combined) * 100.f;
	 
	sigar_close(sigar_cpu);
	return ret;
}

//-------------------------------------------------------------------------------------
float SystemInfo::getCPUPerByPID(uint32 pid)
{
	if(pid == 0)
	{
		pid = (uint32)getProcessPID();
	}

	int status = SIGAR_OK;
	sigar_t *sigarcpulist;
	sigar_cpu_list_t cpulist;
	sigar_open(&sigarcpulist);
	status = sigar_cpu_list_get(sigarcpulist, &cpulist);
	sigar_close(sigarcpulist);

	if (status != SIGAR_OK) 
	{
		printf("error: %d (%s) cpu_list_get(%d)\n",
			   status, sigar_strerror(sigarcpulist, status), pid);

		return 0.f;
	}

    float percent = 0.f;
    sigar_t *sigarproclist;
    sigar_proc_list_t proclist;
    sigar_open(&sigarproclist);
    sigar_proc_list_get(sigarproclist, &proclist);

	if (status != SIGAR_OK) 
	{
		printf("error: %d (%s) cpu_list_get(%d)\n",
			   status, sigar_strerror(sigarproclist, status), pid);

		sigar_close(sigarproclist);
		return 0.f;
	}

	if(!hasPID(pid, &proclist))
	{
		printf("error: not found pid(%d)\n", pid);
		sigar_close(sigarproclist);
		return 0.f;
	}

	// for (size_t i = 0; i < proclist.number; i++)
    {
        sigar_proc_cpu_t cpu;
        status = sigar_proc_cpu_get(sigarproclist, pid, &cpu);

		if (status != SIGAR_OK) 
		{
			printf("error: %d (%s) proc_cpu_get(%d)\n",
				   status, sigar_strerror(sigarproclist, status), pid);

			return 0.f;
		}
    }

    Sleep(1000);

	// for (size_t i = 0; i < proclist.number; i++)
    {
        sigar_proc_cpu_t cpu;
        int status = sigar_proc_cpu_get(sigarproclist, pid, &cpu);
        if (status == SIGAR_OK)
        {
            sigar_proc_state_t procstate;
            status = sigar_proc_state_get(sigarproclist, pid, &procstate);

			if (status != SIGAR_OK) 
			{
				printf("error: %d (%s) proc_state(%d)\n",
					   status, sigar_strerror(sigarproclist, status), pid);

				return 0.f;
			}

            percent = float(cpu.percent) * 100.f / float(cpulist.number);
        }
		else
		{
			printf("error: %d (%s) proc_cpu_get(%d)\n",
				   status, sigar_strerror(sigarproclist, status), pid);

			return 0.f;
		}
    }

    sigar_close(sigarproclist);
	return percent;
}

//-------------------------------------------------------------------------------------
uint64 SystemInfo::getMemUsedByPID(uint32 pid)
{
	if(pid == 0)
	{
		pid = (uint32)getProcessPID();
	}

   int status;
    sigar_t *sigar;
    sigar_proc_list_t proclist;
	sigar_uint64_t total = 0;

    sigar_open(&sigar);

    status = sigar_proc_list_get(sigar, &proclist);

    if (status != SIGAR_OK) {
        printf("proc_list error: %d (%s)\n",
               status, sigar_strerror(sigar, status));
       sigar_close(sigar);
	   return 0;
    }

	if(!hasPID(pid, &proclist))
	{
		printf("error: not found pid(%d)\n", pid);
		sigar_close(sigar);
		return 0;
	}

    //for (i=0; i<(int)proclist.number; i++) 
	{
        sigar_proc_state_t pstate;
        sigar_proc_time_t ptime;

        status = sigar_proc_state_get(sigar, pid, &pstate);
        if (status != SIGAR_OK) 
		{
            printf("error: %d (%s) proc_state(%d)\n",
                   status, sigar_strerror(sigar, status), pid);
			
			goto _END;
        }

        status = sigar_proc_time_get(sigar, pid, &ptime);
        if (status != SIGAR_OK) 
		{
            printf("error: %d (%s) proc_time(%d)\n",
                   status, sigar_strerror(sigar, status), pid);

           goto _END;
        }

		sigar_proc_mem_t proc_mem;
		status = sigar_proc_mem_get(sigar, pid, &proc_mem);

        if (status != SIGAR_OK) 
		{
            printf("error: %d (%s) sigar_proc_mem_get(%d)\n",
                   status, sigar_strerror(sigar, status), pid);
            
			goto _END;
        }

		total = proc_mem.resident;
    }

_END:
    sigar_proc_list_destroy(sigar, &proclist);
    sigar_close(sigar);
	return total;
}

//-------------------------------------------------------------------------------------
} 

