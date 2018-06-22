// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


namespace KBEngine { 
namespace Network
{

INLINE const Address & NetworkInterface::extTcpAddr() const
{
	return extTcpEndpoint_.addr();
}

INLINE const Address & NetworkInterface::extUdpAddr() const
{
	return extUdpEndpoint_.addr();
}

INLINE const Address & NetworkInterface::intTcpAddr() const
{
	return intTcpEndpoint_.addr();
}

INLINE int32 NetworkInterface::numExtChannels() const
{
	return numExtChannels_;
}

}
}
