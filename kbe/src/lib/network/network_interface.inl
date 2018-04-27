// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


namespace KBEngine { 
namespace Network
{

INLINE const Address & NetworkInterface::extaddr() const
{
	return extEndpoint_.addr();
}

INLINE const Address & NetworkInterface::intaddr() const
{
	return intEndpoint_.addr();
}

INLINE int32 NetworkInterface::numExtChannels() const
{
	return numExtChannels_;
}

}
}
