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
