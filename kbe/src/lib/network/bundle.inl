// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


namespace KBEngine { 
namespace Network
{

INLINE bool Bundle::empty() const
{
	return packetsSize() == 0;
}

INLINE int Bundle::packetsSize() const
{
	size_t i = packets_.size();
	if(pCurrPacket_ && !pCurrPacket_->empty())
		++i;

	return (int)i;
}

INLINE void Bundle::currMsgLength(MessageLength1 v)
{
	currMsgLength_ = v;
}

INLINE MessageLength1 Bundle::currMsgLength() const 
{ 
	return currMsgLength_; 
}

INLINE void Bundle::pCurrMsgHandler(const Network::MessageHandler* pMsgHandler)
{ 
	pCurrMsgHandler_ = pMsgHandler; 
	
	if(pCurrMsgHandler_)
		currMsgID_ = pMsgHandler->msgID;
	else
		currMsgID_ = 0;
}

INLINE const Network::MessageHandler* Bundle::pCurrMsgHandler() const
{
	return pCurrMsgHandler_;
}

INLINE Bundle::Packets& Bundle::packets() 
{ 
	return packets_; 
}

INLINE Packet* Bundle::pCurrPacket() const
{ 
	return pCurrPacket_; 
}

INLINE void Bundle::pCurrPacket(Packet* p) 
{ 
	pCurrPacket_ = p; 
}

INLINE MessageID Bundle::messageID() const 
{ 
	return currMsgID_; 
}

INLINE void Bundle::messageID(MessageID id)
{
	currMsgID_ = 0;
}

INLINE int32 Bundle::packetMaxSize() const
{
	return packetMaxSize_;
}

INLINE int32 Bundle::lastPacketSpace()
{
	if (packets_.size() > 0)
	{
		Packet* pPacket = packets_.back();
		if (!pPacket->isEnabledPoolObject())
			return 0;

		return packetMaxSize() - (int32)pPacket->wpos();
	}

	return 0;
}

INLINE bool Bundle::packetHaveSpace()
{
	return isEnabledPoolObject() && lastPacketSpace() > 8;
}

INLINE int32 Bundle::numMessages() const
{ 
	return numMessages_; 
}

INLINE void Bundle::pChannel(Channel* p)
{
	pChannel_= p;
}

INLINE Channel* Bundle::pChannel()
{
	return pChannel_;
}

INLINE void Bundle::finiCurrPacket()
{ 
	if(!pCurrPacket_)
		return;
	
	packets_.push_back(pCurrPacket_); 
	currMsgPacketCount(currMsgPacketCount() + 1);
	pCurrPacket_ = NULL; 
}

INLINE void Bundle::currMsgPacketCount(uint32 v)
{
	currMsgPacketCount_ = v;
}

INLINE uint32 Bundle::currMsgPacketCount() const
{
	return currMsgPacketCount_;
}

INLINE void Bundle::currMsgLengthPos(size_t v)
{
	currMsgLengthPos_ = v;
}

INLINE size_t Bundle::currMsgLengthPos() const
{
	return currMsgLengthPos_;
}

}
}
