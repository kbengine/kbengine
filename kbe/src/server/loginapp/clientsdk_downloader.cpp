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

	Loginapp::getSingleton().networkInterface().dispatcher().cancelTask(this);
}

//-------------------------------------------------------------------------------------
bool ClientSDKDownloader::process()
{
	Network::Channel * pChannel = networkInterface_.findChannel(addr_);

	if (pChannel && sentSize_ < datasize_)
	{
		bool start = true;
		Network::Bundle* pNewBundle = Network::Bundle::createPoolObject(OBJECTPOOL_POINT);
		pNewBundle->newMessage(ClientInterface::onImportClientSDK);
		(*pNewBundle) << start;

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
