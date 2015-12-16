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

#ifndef KBENGINE_SYSINFO_H
#define KBENGINE_SYSINFO_H

#include "debug_helper.h"
#include "common/common.h"
#include "common/singleton.h"

namespace KBEngine
{

class SystemInfo : public Singleton<SystemInfo>
{
public:
	SystemInfo();
	~SystemInfo();

	struct PROCESS_INFOS
	{
		float cpu;
		uint64 memused;
		bool error;
	};

	struct MEM_INFOS
	{
		uint64 total;
		uint64 free;
		uint64 used;
	};

	SystemInfo::MEM_INFOS getMemInfos();
	float getCPUPer();
	float getCPUPerByPID(uint32 pid = 0);
	uint64 getMemUsedByPID(uint32 pid = 0);
	bool hasProcess(uint32 pid);

	uint32 countCPU();

	uint64 totalmem();

	SystemInfo::PROCESS_INFOS getProcessInfo(uint32 pid = 0);

	void update();
	
	void clear();
private:
	bool _autocreate();
	uint64 totalmem_;
};

}

#include "sys_info.inl"

#endif // KBENGINE_SYSINFO_H


