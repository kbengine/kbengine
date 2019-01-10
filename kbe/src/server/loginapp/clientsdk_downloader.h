/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2018 KBEngine.

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

#ifndef KBE_CLIENTSDK_DOWNLOADER_H
#define KBE_CLIENTSDK_DOWNLOADER_H

#include "common/common.h"
#include "network/bundle.h"
#include "network/channel.h"

namespace KBEngine{

class ClientSDKDownloader : public Task
{
public:
	ClientSDKDownloader(Network::NetworkInterface & networkInterface, const Network::Address& addr, size_t clientWindowSize,
		const std::string& assetsPath, const std::string& binPath, const std::string& options);
	~ClientSDKDownloader();
	
	bool process();

private:
	DWORD startWindowsProcessGenSDK(const std::string& file);
	uint16 starLinuxProcessGenSDK(const std::string& file);

	void genSDK();
	bool loadSDKDatas();

	Network::NetworkInterface & networkInterface_;
	Network::Address addr_;
	uint8* datas_;
	size_t datasize_;
	size_t sentSize_;
	size_t clientWindowSize_;
	std::string assetsPath_;
	std::string binPath_;
	std::string options_;
	uint64 lastTime_;
	uint64 startTime_;
	int64 pid_;
	std::vector<std::wstring> sdkFiles_;
	bool loadedSDK_;
	std::wstring currSendFile_;
	std::string out_;

};


}

#endif // KBE_CLIENTSDK_DOWNLOADER_H
