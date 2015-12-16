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

#ifndef KBE_MACHINE_INFOS_H
#define KBE_MACHINE_INFOS_H

#include "common/common.h"
namespace KBEngine{
class MachineInfos : public Singleton<MachineInfos>
{
public:
	MachineInfos();

	const std::string & machineName() const { return machineName_; }
	const std::string & cpuInfo() const { return cpuInfo_; }
	const std::vector<float>& cpuSpeeds() const { return cpuSpeeds_; }
	const std::string & memInfo() const { return memInfo_; }
	const uint64 memTotal() const { return memTotal_; }
	const uint64 memUsed() const { return memUsed_; }
	void updateMem();

private:

#if KBE_PLATFORM != PLATFORM_WIN32
	void fetchLinuxCpuInfo();
	void fetchLinuxMemInfo();
#else
	void fetchWindowsCpuInfo();
	void fetchWindowsMemInfo();
#endif

	std::string machineName_;

	std::string cpuInfo_;
	std::vector<float> cpuSpeeds_;

	std::string memInfo_;
	uint64 memTotal_;
	uint64 memUsed_;
};
}
#endif // KBE_MACHINE_INFOS_H
