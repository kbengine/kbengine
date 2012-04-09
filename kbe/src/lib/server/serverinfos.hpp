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

	const std::string & serverName() const { return m_serverName_; }
	const std::string & cpuInfo() const { return m_cpuInfo_; }
	const std::vector<float>& cpuSpeeds() const { return m_cpuSpeeds_; }
	const std::string & memInfo() const { return m_memInfo_; }
	const uint64 memTotal() const { return m_memTotal_; }
	const uint64 memUsed() const { return m_memUsed_; }
	void updateMem();
private:

#if KBE_PLATFORM != PLATFORM_WIN32
	void fetchLinuxCpuInfo();
	void fetchLinuxMemInfo();
#endif

	std::string m_serverName_;

	std::string m_cpuInfo_;
	std::vector<float> m_cpuSpeeds_;

	std::string m_memInfo_;
	uint64 m_memTotal_;
	uint64 m_memUsed_;
};
}
#endif // __SERVER_INFOS_HPP__
