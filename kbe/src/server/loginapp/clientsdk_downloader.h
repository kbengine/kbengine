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
	ClientSDKDownloader(Network::NetworkInterface & networkInterface, const Network::Address& addr, size_t clientWindowSize, uint8* datas, size_t datasize);
	~ClientSDKDownloader();
	
	bool process();

private:
	Network::NetworkInterface & networkInterface_;
	Network::Address addr_;
	uint8* datas_;
	size_t datasize_;
	size_t sentSize_;
	size_t clientWindowSize_;
};


}

#endif // KBE_CLIENTSDK_DOWNLOADER_H
