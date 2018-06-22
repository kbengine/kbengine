// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

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
