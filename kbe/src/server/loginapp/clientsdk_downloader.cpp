// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "loginapp.h"
#include "clientsdk_downloader.h"
#include "server/serverconfig.h"

#include "client_lib/client_interface.h"

namespace KBEngine{	


//-------------------------------------------------------------------------------------
ClientSDKDownloader::ClientSDKDownloader(Network::NetworkInterface & networkInterface, const Network::Address& addr, size_t clientWindowSize, uint8* datas, size_t datasize):
networkInterface_(networkInterface),
addr_(addr),
datas_(datas),
datasize_(datasize),
sentSize_(0),
clientWindowSize_(clientWindowSize)
{
	Loginapp::getSingleton().networkInterface().dispatcher().addTask(this);

	if (clientWindowSize_ <= 0)
		clientWindowSize_ = 1024;

	if (clientWindowSize_ > datasize)
		clientWindowSize_ = datasize;
}

//-------------------------------------------------------------------------------------
ClientSDKDownloader::~ClientSDKDownloader()
{
	DEBUG_MSG(fmt::format("ClientSDKDownloader::~ClientSDKDownloader(): sent {} bytes! addr={}\n", sentSize_, addr_.c_str()));

	if(datas_)
		free(datas_);

	if(sentSize_ < datasize_)
		Loginapp::getSingleton().networkInterface().dispatcher().cancelTask(this);
}

//-------------------------------------------------------------------------------------
bool ClientSDKDownloader::process()
{
	Network::Channel * pChannel = networkInterface_.findChannel(addr_);

	if (pChannel && sentSize_ < datasize_)
	{
		Network::Bundle* pNewBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
		pNewBundle->newMessage(ClientInterface::onImportClientSDK);
		(*pNewBundle) << (int)datasize_;

		pNewBundle->appendBlob(datas_, clientWindowSize_);
		pChannel->send(pNewBundle);

		sentSize_ += clientWindowSize_;
		if (clientWindowSize_ > datasize_ - sentSize_)
			clientWindowSize_ = datasize_ - sentSize_;

		return true;
	}

	delete this;
	return false;
}

//-------------------------------------------------------------------------------------

}
