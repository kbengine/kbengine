/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 kbegine Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
*/

#ifndef __SERVER_INFOS_HPP__
#define __SERVER_INFOS_HPP__

#include "cstdkbe/cstdkbe.hpp"
namespace KBEngine{
class ServerInfos : public Singleton<ServerInfos>
{
public:
	ServerInfos();

	const std::string & serverName() const { return serverName_; }
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

	std::string serverName_;

	std::string cpuInfo_;
	std::vector<float> cpuSpeeds_;

	std::string memInfo_;
	uint64 memTotal_;
	uint64 memUsed_;
};
}
#endif // __SERVER_INFOS_HPP__
