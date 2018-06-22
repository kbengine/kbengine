// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


namespace KBEngine { 
namespace Network
{

INLINE const Address & Channel::addr() const
{
	return pEndPoint_->addr();
}

INLINE EndPoint * Channel::pEndPoint() const
{
	return pEndPoint_;
}

INLINE PacketReader* Channel::pPacketReader() const
{
	return pPacketReader_;
}

INLINE PacketReceiver* Channel::pPacketReceiver() const
{
	return pPacketReceiver_;
}

INLINE void Channel::pPacketReceiver(PacketReceiver* pPacketReceiver)
{
	pPacketReceiver_ = pPacketReceiver;
}

INLINE PacketSender* Channel::pPacketSender() const
{
	return pPacketSender_;
}

INLINE void Channel::pPacketSender(PacketSender* pPacketSender)
{
	pPacketSender_ = pPacketSender;
}

INLINE void Channel::pushBundle(Bundle* pBundle)
{
	bundles_.push_back(pBundle);
}

}
}
