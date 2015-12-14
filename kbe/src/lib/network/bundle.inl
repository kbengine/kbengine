/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2016 KBEngine.

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

INLINE MessageLength Bundle::currMsgLength() const 
{ 
	return currMsgLength_; 
}
	
INLINE void Bundle::pCurrMsgHandler(const Network::MessageHandler* pMsgHandler)
{ 
	pCurrMsgHandler_ = pMsgHandler; 
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

INLINE int32 Bundle::numMessages() const
{ 
	return numMessages_; 
}

INLINE void Bundle::finiCurrPacket(){ 
	packets_.push_back(pCurrPacket_); 
	pCurrPacket_ = NULL; 
}
}
}
