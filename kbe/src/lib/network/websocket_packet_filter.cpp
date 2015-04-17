/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 KBEngine.

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


#include "websocket_packet_filter.h"

#include "network/bundle.h"
#include "network/channel.h"
#include "network/tcp_packet.h"
#include "network/network_interface.h"
#include "network/packet_receiver.h"

namespace KBEngine { 
namespace Network
{

uint64 hl64ton(uint64 host)   
{   
	uint64   ret = 0;   
	ulong   high,low;

	low = host & 0xFFFFFFFF;
	high = (host >> 32) & 0xFFFFFFFF;
	low = htonl(low);   
	high = htonl(high);   
	ret = low;
	ret <<= 32;   
	ret |= high;   
	return ret;   
}

uint64 ntohl64(uint64 host)   
{   
	uint64   ret = 0;   
	ulong   high,low;

	low = host & 0xFFFFFFFF;
	high = (host >> 32) & 0xFFFFFFFF;
	low = ntohl(low);   
	high = ntohl(high);   

	ret = low;
	ret <<= 32;   
	ret |= high;   
	return ret;   
}

//-------------------------------------------------------------------------------------
WebSocketPacketFilter::WebSocketPacketFilter(Channel* pChannel):
	web_pFragmentDatasRemain_(2),
	web_fragmentDatasFlag_(FRAGMENT_DATA_BASIC_LENGTH),
	basicSize_(0),
	payloadSize_(0),
	payloadTotalSize_(0),
	data_xy_(),
	pChannel_(pChannel),
	pTCPPacket_(NULL)
{
}

//-------------------------------------------------------------------------------------
WebSocketPacketFilter::~WebSocketPacketFilter()
{
	TCPPacket::ObjPool().reclaimObject(pTCPPacket_);
	pTCPPacket_ = NULL;
}

//-------------------------------------------------------------------------------------
Reason WebSocketPacketFilter::send(Channel * pChannel, PacketSender& sender, Packet * pPacket)
{
	TCPPacket* pRetTCPPacket = TCPPacket::ObjPool().createObject();

	Bundle* pBundle = pPacket->pBundle();
	
	uint64 payloadSize = pPacket->length();
	int8 basicSize = (int8)payloadSize;

	//create the flags byte
	uint8 payloadFlags = BINARY_FRAME;

	if(pBundle && pBundle->packets().size() > 1)
	{
		bool isEnd = pBundle->packets().back() == pPacket;
		bool isBegin = pBundle->packets().front() == pPacket;

		if(!isEnd && !isBegin)
		{
			payloadFlags = NEXT_FRAME;
		}
		else
		{
			if(!isEnd)
				payloadFlags = BEGIN_BINARY_FRAME;
			else
				payloadFlags = END_FRAME;
		}
	}

	(*pRetTCPPacket) << payloadFlags;

	if(payloadSize <= 125)
	{
		(*pRetTCPPacket) << basicSize;
	}
	else if (payloadSize <= 65535)
	{
		basicSize = 126;
		(*pRetTCPPacket) << basicSize;

		uint8 len[2];
		len[0] = ( payloadSize >> 8 ) & 0xff;
		len[1] = ( payloadSize ) & 0xff;
		(*pRetTCPPacket).append((uint8*)&len, 2);
	}
	else
	{
		basicSize = 127;
		(*pRetTCPPacket) << basicSize;
		MemoryStreamConverter::apply<uint64>(&payloadSize);
		(*pRetTCPPacket) << payloadSize;
	}

	int space = pPacket->length() - pRetTCPPacket->space();
	if(space > 0)
	{
		WARNING_MSG(fmt::format("WebSocketPacketFilter::send: no free space, buffer added:{}, total={}.\n",
			space, pRetTCPPacket->size()));

		pRetTCPPacket->data_resize(pRetTCPPacket->size() + space);
	}

	(*pRetTCPPacket).append(pPacket->data() + pPacket->rpos(), pPacket->length());
	
	pRetTCPPacket->swap(*(static_cast<KBEngine::MemoryStream*>(pPacket)));
	TCPPacket::ObjPool().reclaimObject(pRetTCPPacket);
	return PacketFilter::send(pChannel, sender, pPacket);
}

//-------------------------------------------------------------------------------------
Reason WebSocketPacketFilter::recv(Channel * pChannel, PacketReceiver & receiver, Packet * pPacket)
{
	TCPPacket* pRetTCPPacket = NULL;

	while(pPacket->length() > 0)
	{
		if(web_fragmentDatasFlag_ != FRAGMENT_DATA_MESSAGE_BODY)
		{
			if(web_fragmentDatasFlag_ == FRAGMENT_DATA_BASIC_LENGTH)
			{
				if(web_pFragmentDatasRemain_ == 2)
				{
					if(pPacket->length() == 1)
					{
						web_pFragmentDatasRemain_ -= 1;
						pPacket->done();

						pRetTCPPacket = pTCPPacket_;
						pTCPPacket_ = NULL;
						break;
					}
					else
					{
						(*pPacket) >> basicSize_;
						if(basicSize_ != BINARY_FRAME)
						{
							ERROR_MSG(fmt::format("WebSocketPacketFilter::recv: frame_opcode({}) != {}, addr={}\n", 
								basicSize_, BINARY_FRAME, pChannel->c_str()));

							this->pChannel_->condemn();
							return REASON_WEBSOCKET_ERROR;
						}
					}
				}

				(*pPacket) >> basicSize_;
				basicSize_ &= 0x7f;

				switch(basicSize_)
				{
				case 126:
					web_pFragmentDatasRemain_ = 2;
					web_fragmentDatasFlag_ = FRAGMENT_DATA_PAYLOAD_LENGTH;
					break;
				case 127:
					web_pFragmentDatasRemain_ = 8;
					web_fragmentDatasFlag_ = FRAGMENT_DATA_PAYLOAD_LENGTH;
					break;
				default:
					if(basicSize_ > 127)
					{
						this->pChannel_->condemn();

						ERROR_MSG(fmt::format("WebSocketPacketReader::processMessages: basicSize({}) is error! addr={}!\n",
							basicSize_, pChannel_->c_str()));

						return REASON_WEBSOCKET_ERROR;
					}

					web_pFragmentDatasRemain_ = 4;
					payloadSize_ = basicSize_;
					payloadTotalSize_ = payloadSize_;
					web_fragmentDatasFlag_ = FRAGMENT_DATA_PAYLOAD_MASKS;
					break;
				};
			}

			if(web_fragmentDatasFlag_ == FRAGMENT_DATA_PAYLOAD_LENGTH)
			{
				if(pPacket->length() >= web_pFragmentDatasRemain_)
				{
					memcpy(&masks_[(basicSize_ == 126 ? 2 : 8) - web_pFragmentDatasRemain_], pPacket->data() + pPacket->rpos(), web_pFragmentDatasRemain_);
					pPacket->read_skip(web_pFragmentDatasRemain_);
					web_pFragmentDatasRemain_ = 4;
					web_fragmentDatasFlag_ = FRAGMENT_DATA_PAYLOAD_MASKS;

					switch(basicSize_)
					{
					case 126:
						payloadSize_ = ntohs( *(u_short*) (masks_) );
						break;
					case 127:
						payloadSize_ = ntohl64( *(uint64*) (masks_) );
						break;
					default:
						payloadSize_ = basicSize_;
						break;
					};

					payloadTotalSize_ = payloadSize_;

				}
				else
				{
					memcpy(&masks_[(basicSize_ == 126 ? 2 : 8) - web_pFragmentDatasRemain_], pPacket->data() + pPacket->rpos(), pPacket->length());
					web_pFragmentDatasRemain_ -= pPacket->length();
					pPacket->done();
					pRetTCPPacket = pTCPPacket_;
					pTCPPacket_ = NULL;
					break;
				}
			}

			if(web_fragmentDatasFlag_ == FRAGMENT_DATA_PAYLOAD_MASKS)
			{
				if(pPacket->length() >= web_pFragmentDatasRemain_)
				{
					memcpy(&masks_[4 - web_pFragmentDatasRemain_], pPacket->data() + pPacket->rpos(), web_pFragmentDatasRemain_);
					pPacket->read_skip(web_pFragmentDatasRemain_);
					web_pFragmentDatasRemain_ = 0;
					web_fragmentDatasFlag_ = FRAGMENT_DATA_MESSAGE_BODY;
					pTCPPacket_ = TCPPacket::ObjPool().createObject();
				}
				else
				{
					memcpy(&masks_[4 - web_pFragmentDatasRemain_], pPacket->data() + pPacket->rpos(), pPacket->length());
					web_pFragmentDatasRemain_ -= pPacket->length();
					pPacket->done();
					pRetTCPPacket = pTCPPacket_;
					pTCPPacket_ = NULL;
					break;
				}
			}
		}
		else
		{
			if(payloadSize_ == 0)
			{
				web_fragmentDatasFlag_ = FRAGMENT_DATA_BASIC_LENGTH;
				web_pFragmentDatasRemain_ = 2;
				pRetTCPPacket = pTCPPacket_;
				pTCPPacket_ = NULL;
				break;
			}
			else
			{
				if(pPacket->length() >= payloadSize_)
				{
					pTCPPacket_->append(pPacket->data() + pPacket->rpos(), (size_t)payloadSize_);

					web_fragmentDatasFlag_ = FRAGMENT_DATA_BASIC_LENGTH;
					web_pFragmentDatasRemain_ = 2;
					pPacket->read_skip((size_t)payloadSize_);
					uint64 startSize = payloadTotalSize_ - payloadSize_;
					payloadSize_ = 0;

					for(uint64 i=0; i<pTCPPacket_->length(); ++i)
					{
						pTCPPacket_->data()[i] = (pTCPPacket_->data()[i] ^ masks_[(i + startSize) % 4]);
					}

					web_fragmentDatasFlag_ = FRAGMENT_DATA_BASIC_LENGTH;
					web_pFragmentDatasRemain_ = 2;
					pRetTCPPacket = pTCPPacket_;
					pTCPPacket_ = NULL;

					if(pPacket->length() == 0)
					{
						break;
					}
					else
					{
						Reason reason = PacketFilter::recv(pChannel, receiver, pRetTCPPacket);
						if(REASON_SUCCESS != reason)
						{
							return reason;
						}

						pRetTCPPacket = NULL;
					}
				}
				else
				{
					pTCPPacket_->append(pPacket->data() + pPacket->rpos(), pPacket->length());

					uint64 startSize = payloadTotalSize_ - payloadSize_;
					payloadSize_ -= pPacket->length();
					pPacket->done();
					pRetTCPPacket = pTCPPacket_;
					
					for(uint64 i=0; i<pTCPPacket_->length(); ++i)
					{
						pTCPPacket_->data()[i] = (pTCPPacket_->data()[i] ^ masks_[(i + startSize) % 4]);
					}

					pTCPPacket_ = TCPPacket::ObjPool().createObject();
					break;
				}
			}
		}
	}

	return PacketFilter::recv(pChannel, receiver, pRetTCPPacket);
}

//-------------------------------------------------------------------------------------
} 
}
