// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "watcher.h"
#include "watch_pools.h"
#include "network/bundle.h"
#include "network/address.h"
#include "network/endpoint.h"
#include "network/tcp_packet.h"
#include "network/udp_packet.h"
#include "network/tcp_packet_receiver.h"
#include "network/udp_packet_receiver.h"

namespace KBEngine { 

//-------------------------------------------------------------------------------------
int32 watchBundlePool_size()
{
	return (int)Network::Bundle::ObjPool().objects().size();
}

int32 watchBundlePool_max()
{
	return (int)Network::Bundle::ObjPool().max();
}

int32 watchBundlePool_totalAllocs()
{
	return (int)Network::Bundle::ObjPool().totalAllocs();
}

bool watchBundlePool_isDestroyed()
{
	return Network::Bundle::ObjPool().isDestroyed();
}

uint32 watchBundlePool_bytes()
{
	size_t bytes = 0;

	ObjectPool<Network::Bundle>::OBJECTS::const_iterator iter = Network::Bundle::ObjPool().objects().begin();
	for(; iter != Network::Bundle::ObjPool().objects().end(); ++iter)
	{
		bytes += static_cast<PoolObject*>((*iter))->getPoolObjectBytes();
	}

	return (uint32)bytes;
}

//-------------------------------------------------------------------------------------
int32 watchAddressPool_size()
{
	return (int)Network::Address::ObjPool().objects().size();
}

int32 watchAddressPool_max()
{
	return (int)Network::Address::ObjPool().max();
}

int32 watchAddressPool_totalAllocs()
{
	return (int)Network::Address::ObjPool().totalAllocs();
}

bool watchAddressPool_isDestroyed()
{
	return Network::Address::ObjPool().isDestroyed();
}

uint32 watchAddressPool_bytes()
{
	size_t bytes = 0;

	ObjectPool<Network::Address>::OBJECTS::const_iterator iter = Network::Address::ObjPool().objects().begin();
	for(; iter != Network::Address::ObjPool().objects().end(); ++iter)
	{
		bytes += static_cast<PoolObject*>((*iter))->getPoolObjectBytes();
	}

	return (uint32)bytes;
}

//-------------------------------------------------------------------------------------
int32 watchMemoryStreamPool_size()
{
	return (int)MemoryStream::ObjPool().objects().size();
}

int32 watchMemoryStreamPool_max()
{
	return (int)MemoryStream::ObjPool().max();
}

int32 watchMemoryStreamPool_totalAllocs()
{
	return (int)MemoryStream::ObjPool().totalAllocs();
}

bool watchMemoryStreamPool_isDestroyed()
{
	return MemoryStream::ObjPool().isDestroyed();
}

uint32 watchMemoryStreamPool_bytes()
{
	size_t bytes = 0;

	ObjectPool<MemoryStream>::OBJECTS::const_iterator iter = MemoryStream::ObjPool().objects().begin();
	for(; iter != MemoryStream::ObjPool().objects().end(); ++iter)
	{
		bytes += static_cast<PoolObject*>((*iter))->getPoolObjectBytes();
	}

	return (uint32)bytes;
}

//-------------------------------------------------------------------------------------
int32 watchTCPPacketPool_size()
{
	return (int)Network::TCPPacket::ObjPool().objects().size();
}

int32 watchTCPPacketPool_max()
{
	return (int)Network::TCPPacket::ObjPool().max();
}

int32 watchTCPPacketPool_totalAllocs()
{
	return (int)Network::TCPPacket::ObjPool().totalAllocs();
}

bool watchTCPPacketPool_isDestroyed()
{
	return Network::TCPPacket::ObjPool().isDestroyed();
}

uint32 watchTCPPacketPool_bytes()
{
	size_t bytes = 0;

	ObjectPool<Network::TCPPacket>::OBJECTS::const_iterator iter = Network::TCPPacket::ObjPool().objects().begin();
	for(; iter != Network::TCPPacket::ObjPool().objects().end(); ++iter)
	{
		bytes += static_cast<PoolObject*>((*iter))->getPoolObjectBytes();
	}

	return (uint32)bytes;
}

//-------------------------------------------------------------------------------------
int32 watchTCPPacketReceiverPool_size()
{
	return (int)Network::TCPPacketReceiver::ObjPool().objects().size();
}

int32 watchTCPPacketReceiverPool_max()
{
	return (int)Network::TCPPacketReceiver::ObjPool().max();
}

int32 watchTCPPacketReceiverPool_totalAllocs()
{
	return (int)Network::TCPPacketReceiver::ObjPool().totalAllocs();
}

bool watchTCPPacketReceiverPool_isDestroyed()
{
	return Network::TCPPacketReceiver::ObjPool().isDestroyed();
}

uint32 watchTCPPacketReceiverPool_bytes()
{
	size_t bytes = 0;

	ObjectPool<Network::TCPPacketReceiver>::OBJECTS::const_iterator iter = Network::TCPPacketReceiver::ObjPool().objects().begin();
	for(; iter != Network::TCPPacketReceiver::ObjPool().objects().end(); ++iter)
	{
		bytes += static_cast<PoolObject*>((*iter))->getPoolObjectBytes();
	}

	return (uint32)bytes;
}

//-------------------------------------------------------------------------------------
int32 watchUDPPacketPool_size()
{
	return (int)Network::UDPPacket::ObjPool().objects().size();
}

int32 watchUDPPacketPool_max()
{
	return (int)Network::UDPPacket::ObjPool().max();
}

int32 watchUDPPacketPool_totalAllocs()
{
	return (int)Network::UDPPacket::ObjPool().totalAllocs();
}

bool watchUDPPacketPool_isDestroyed()
{
	return Network::UDPPacket::ObjPool().isDestroyed();
}

uint32 watchUDPPacketPool_bytes()
{
	size_t bytes = 0;

	ObjectPool<Network::UDPPacket>::OBJECTS::const_iterator iter = Network::UDPPacket::ObjPool().objects().begin();
	for(; iter != Network::UDPPacket::ObjPool().objects().end(); ++iter)
	{
		bytes += static_cast<PoolObject*>((*iter))->getPoolObjectBytes();
	}

	return (uint32)bytes;
}

//-------------------------------------------------------------------------------------
int32 watchUDPPacketReceiverPool_size()
{
	return (int)Network::UDPPacketReceiver::ObjPool().objects().size();
}

int32 watchUDPPacketReceiverPool_max()
{
	return (int)Network::UDPPacketReceiver::ObjPool().max();
}

int32 watchUDPPacketReceiverPool_totalAllocs()
{
	return (int)Network::UDPPacketReceiver::ObjPool().totalAllocs();
}

bool watchUDPPacketReceiverPool_isDestroyed()
{
	return Network::UDPPacketReceiver::ObjPool().isDestroyed();
}

uint32 watchUDPPacketReceiverPool_bytes()
{
	size_t bytes = 0;

	ObjectPool<Network::UDPPacketReceiver>::OBJECTS::const_iterator iter = Network::UDPPacketReceiver::ObjPool().objects().begin();
	for(; iter != Network::UDPPacketReceiver::ObjPool().objects().end(); ++iter)
	{
		bytes += static_cast<PoolObject*>((*iter))->getPoolObjectBytes();
	}

	return (uint32)bytes;
}

//-------------------------------------------------------------------------------------
int32 watchEndPointPool_size()
{
	return (int)Network::EndPoint::ObjPool().objects().size();
}

int32 watchEndPointPool_max()
{
	return (int)Network::EndPoint::ObjPool().max();
}

int32 watchEndPointPool_totalAllocs()
{
	return (int)Network::EndPoint::ObjPool().totalAllocs();
}

bool watchEndPointPool_isDestroyed()
{
	return Network::EndPoint::ObjPool().isDestroyed();
}

uint32 watchEndPointPool_bytes()
{
	size_t bytes = 0;

	ObjectPool<Network::EndPoint>::OBJECTS::const_iterator iter = Network::EndPoint::ObjPool().objects().begin();
	for(; iter != Network::EndPoint::ObjPool().objects().end(); ++iter)
	{
		bytes += static_cast<PoolObject*>((*iter))->getPoolObjectBytes();
	}

	return (uint32)bytes;
}

//-------------------------------------------------------------------------------------
int32 watchChannelPool_size()
{
	return (int)Network::Channel::ObjPool().objects().size();
}

int32 watchChannelPool_max()
{
	return (int)Network::Channel::ObjPool().max();
}

int32 watchChannelPool_totalAllocs()
{
	return (int)Network::Channel::ObjPool().totalAllocs();
}

bool watchChannelPool_isDestroyed()
{
	return Network::Channel::ObjPool().isDestroyed();
}

uint32 watchChannelPool_bytes()
{
	size_t bytes = 0;

	ObjectPool<Network::Channel>::OBJECTS::const_iterator iter = Network::Channel::ObjPool().objects().begin();
	for(; iter != Network::Channel::ObjPool().objects().end(); ++iter)
	{
		bytes += static_cast<PoolObject*>((*iter))->getPoolObjectBytes();
	}

	return (uint32)bytes;
}

//-------------------------------------------------------------------------------------
std::string watch_tracelogs()
{
	WatcherPaths::WATCHER_PATHS paths = WatcherPaths::root().watcherPaths()["root"]->watcherPaths();
	WatcherPaths::WATCHER_PATHS::iterator iter = paths.find("objectPools");

	if (iter == paths.end())
		return "NotFound";

	paths = iter->second->watcherPaths();
	iter = paths.begin();

	for (; iter != paths.end(); ++iter)
	{
		const std::string& pathName = iter->first;
		std::map<std::string, ObjectPoolLogPoint>* pLogPoints = NULL;

		if (pathName == "Bundle")
		{
			pLogPoints = &Network::Bundle::ObjPool().logPoints();
		}
		else if (pathName == "Address")
		{
			pLogPoints = &Network::Address::ObjPool().logPoints();
		}
		else if (pathName == "MemoryStream")
		{
			pLogPoints = &MemoryStream::ObjPool().logPoints();
		}
		else if (pathName == "TCPPacket")
		{
			pLogPoints = &Network::TCPPacket::ObjPool().logPoints();
		}
		else if (pathName == "TCPPacketReceiver")
		{
			pLogPoints = &Network::TCPPacketReceiver::ObjPool().logPoints();
		}
		else if (pathName == "UDPPacket")
		{
			pLogPoints = &Network::UDPPacket::ObjPool().logPoints();
		}
		else if (pathName == "UDPPacketReceiver")
		{
			pLogPoints = &Network::UDPPacketReceiver::ObjPool().logPoints();
		}
		else if (pathName == "EndPoint")
		{
			pLogPoints = &Network::EndPoint::ObjPool().logPoints();
		}
		else if (pathName == "Channel")
		{
			pLogPoints = &Network::Channel::ObjPool().logPoints();
		}

		if (!pLogPoints)
			continue;

		std::map<std::string, ObjectPoolLogPoint>::const_iterator oiter = pLogPoints->begin();
		for (; oiter != pLogPoints->end(); ++oiter)
		{
			const std::string& pointName = oiter->first;

			Watchers::WATCHER_MAP& watchers = iter->second->watchers().watcherObjs();
			Watchers::WATCHER_MAP::iterator fiter = watchers.find(pointName);
			if (fiter != watchers.end())
				continue;

			WATCH_OBJECT(fmt::format("objectPools/{}/{}", pathName, pointName).c_str(), oiter->second.count);
		}
	}

	return "Collecting...";
}

//-------------------------------------------------------------------------------------
bool WatchPool::initWatchPools()
{
	WATCH_OBJECT("objectPools/Bundle/size", &watchBundlePool_size);
	WATCH_OBJECT("objectPools/Bundle/max", &watchBundlePool_max);
	WATCH_OBJECT("objectPools/Bundle/isDestroyed", &watchBundlePool_isDestroyed);
	WATCH_OBJECT("objectPools/Bundle/memory", &watchBundlePool_bytes);
	WATCH_OBJECT("objectPools/Bundle/totalAllocs", &watchBundlePool_totalAllocs);

	WATCH_OBJECT("objectPools/Address/size", &watchAddressPool_size);
	WATCH_OBJECT("objectPools/Address/max", &watchAddressPool_max);
	WATCH_OBJECT("objectPools/Address/isDestroyed", &watchAddressPool_isDestroyed);
	WATCH_OBJECT("objectPools/Address/memory", &watchAddressPool_bytes);
	WATCH_OBJECT("objectPools/Address/totalAllocs", &watchAddressPool_totalAllocs);

	WATCH_OBJECT("objectPools/MemoryStream/size", &watchMemoryStreamPool_size);
	WATCH_OBJECT("objectPools/MemoryStream/max", &watchMemoryStreamPool_max);
	WATCH_OBJECT("objectPools/MemoryStream/isDestroyed", &watchMemoryStreamPool_isDestroyed);
	WATCH_OBJECT("objectPools/MemoryStream/memory", &watchMemoryStreamPool_bytes);
	WATCH_OBJECT("objectPools/MemoryStream/totalAllocs", &watchMemoryStreamPool_totalAllocs);

	WATCH_OBJECT("objectPools/TCPPacket/size", &watchTCPPacketPool_size);
	WATCH_OBJECT("objectPools/TCPPacket/max", &watchTCPPacketPool_max);
	WATCH_OBJECT("objectPools/TCPPacket/isDestroyed", &watchTCPPacketPool_isDestroyed);
	WATCH_OBJECT("objectPools/TCPPacket/memory", &watchTCPPacketPool_bytes);
	WATCH_OBJECT("objectPools/TCPPacket/totalAllocs", &watchTCPPacketPool_totalAllocs);

	WATCH_OBJECT("objectPools/TCPPacketReceiver/size", &watchTCPPacketReceiverPool_size);
	WATCH_OBJECT("objectPools/TCPPacketReceiver/max", &watchTCPPacketReceiverPool_max);
	WATCH_OBJECT("objectPools/TCPPacketReceiver/isDestroyed", &watchTCPPacketReceiverPool_isDestroyed);
	WATCH_OBJECT("objectPools/TCPPacketReceiver/memory", &watchTCPPacketReceiverPool_bytes);
	WATCH_OBJECT("objectPools/TCPPacketReceiver/totalAllocs", &watchTCPPacketReceiverPool_totalAllocs);

	WATCH_OBJECT("objectPools/UDPPacket/size", &watchUDPPacketPool_size);
	WATCH_OBJECT("objectPools/UDPPacket/max", &watchUDPPacketPool_max);
	WATCH_OBJECT("objectPools/UDPPacket/isDestroyed", &watchUDPPacketPool_isDestroyed);
	WATCH_OBJECT("objectPools/UDPPacket/memory", &watchUDPPacketPool_bytes);
	WATCH_OBJECT("objectPools/UDPPacket/totalAllocs", &watchUDPPacketPool_totalAllocs);

	WATCH_OBJECT("objectPools/UDPPacketReceiver/size", &watchUDPPacketReceiverPool_size);
	WATCH_OBJECT("objectPools/UDPPacketReceiver/max", &watchUDPPacketReceiverPool_max);
	WATCH_OBJECT("objectPools/UDPPacketReceiver/isDestroyed", &watchUDPPacketReceiverPool_isDestroyed);
	WATCH_OBJECT("objectPools/UDPPacketReceiver/memory", &watchUDPPacketReceiverPool_bytes);
	WATCH_OBJECT("objectPools/UDPPacketReceiver/totalAllocs", &watchUDPPacketReceiverPool_totalAllocs);

	WATCH_OBJECT("objectPools/EndPoint/size", &watchEndPointPool_size);
	WATCH_OBJECT("objectPools/EndPoint/max", &watchEndPointPool_max);
	WATCH_OBJECT("objectPools/EndPoint/isDestroyed", &watchEndPointPool_isDestroyed);
	WATCH_OBJECT("objectPools/EndPoint/memory", &watchEndPointPool_bytes);
	WATCH_OBJECT("objectPools/EndPoint/totalAllocs", &watchEndPointPool_totalAllocs);

	WATCH_OBJECT("objectPools/Channel/size", &watchChannelPool_size);
	WATCH_OBJECT("objectPools/Channel/max", &watchChannelPool_max);
	WATCH_OBJECT("objectPools/Channel/isDestroyed", &watchChannelPool_isDestroyed);
	WATCH_OBJECT("objectPools/Channel/memory", &watchChannelPool_bytes);
	WATCH_OBJECT("objectPools/Channel/totalAllocs", &watchChannelPool_totalAllocs);

	WATCH_OBJECT("objectPools/TraceLogs", &watch_tracelogs);
	return true;
}

//-------------------------------------------------------------------------------------
bool WatchPool::finiWatchPools()
{
	return true;
}

//-------------------------------------------------------------------------------------

}
