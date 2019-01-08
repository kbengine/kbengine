// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

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
