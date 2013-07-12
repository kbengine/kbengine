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


#include "html5_packet_filter.hpp"

#include "network/bundle.hpp"
#include "network/channel.hpp"
#include "network/tcp_packet.hpp"
#include "network/network_interface.hpp"
#include "network/packet_receiver.hpp"

namespace KBEngine { 
namespace Mercury
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
HTML5PacketFilter::HTML5PacketFilter(Channel* pChannel):
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
HTML5PacketFilter::~HTML5PacketFilter()
{
	TCPPacket::ObjPool().reclaimObject(pTCPPacket_);
	pTCPPacket_ = NULL;
}

//-------------------------------------------------------------------------------------
Reason HTML5PacketFilter::send(NetworkInterface & networkInterface, Channel * pChannel, Packet * pPacket)
{
	TCPPacket* pRetTCPPacket = TCPPacket::ObjPool().createObject();

	Bundle* pBundle = pPacket->pBundle();
	
	uint64 payloadSize = pPacket->totalSize();
	int8 basicSize = payloadSize;

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

	int space = pPacket->opsize() - pRetTCPPacket->fillfree();
	if(space > 0)
	{
		WARNING_MSG(boost::format("HTML5PacketFilter::send: no free space, buffer added:%1%, total=%2%.\n") % 
			space % pRetTCPPacket->size());

		pRetTCPPacket->data_resize(pRetTCPPacket->size() + space);
	}

	(*pRetTCPPacket).append(pPacket->data() + pPacket->rpos(), pPacket->opsize());
	
	pRetTCPPacket->swap(*(static_cast<KBEngine::MemoryStream*>(pPacket)));
	TCPPacket::ObjPool().reclaimObject(pRetTCPPacket);
	
	return PacketFilter::send(networkInterface, pChannel, pPacket);
}

//-------------------------------------------------------------------------------------
Reason HTML5PacketFilter::recv(Channel * pChannel, PacketReceiver & receiver, Packet * pPacket)
{
	TCPPacket* pRetTCPPacket = NULL;

	while(pPacket->totalSize() > 0)
	{
		if(web_fragmentDatasFlag_ != FRAGMENT_DATA_MESSAGE_BODY)
		{
			if(web_fragmentDatasFlag_ == FRAGMENT_DATA_BASIC_LENGTH)
			{
				if(web_pFragmentDatasRemain_ == 2)
				{
					if(pPacket->totalSize() == 1)
					{
						web_pFragmentDatasRemain_ -= 1;
						pPacket->opfini();

						pRetTCPPacket = pTCPPacket_;
						pTCPPacket_ = NULL;
						break;
					}
					else
					{
						(*pPacket) >> basicSize_;
						if(basicSize_ != BINARY_FRAME)
						{
							ERROR_MSG(boost::format("HTML5PacketFilter::recv: frame_opcode(%1%) != %2%, addr=%3%\n") % 
								basicSize_ % BINARY_FRAME % pChannel->c_str());

							this->pChannel_->condemn();
							return REASON_HTML5_ERROR;
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
						ERROR_MSG(boost::format("HTML5PacketReader::processMessages: basicSize(%1%) is error! addr=%2%!\n") % 
							basicSize_ % pChannel_->c_str());
						return REASON_HTML5_ERROR;
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
				if(pPacket->totalSize() >= web_pFragmentDatasRemain_)
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
					memcpy(&masks_[(basicSize_ == 126 ? 2 : 8) - web_pFragmentDatasRemain_], pPacket->data() + pPacket->rpos(), pPacket->totalSize());
					web_pFragmentDatasRemain_ -= pPacket->totalSize();
					pPacket->opfini();
					pRetTCPPacket = pTCPPacket_;
					pTCPPacket_ = NULL;
					break;
				}
			}

			if(web_fragmentDatasFlag_ == FRAGMENT_DATA_PAYLOAD_MASKS)
			{
				if(pPacket->totalSize() >= web_pFragmentDatasRemain_)
				{
					memcpy(&masks_[4 - web_pFragmentDatasRemain_], pPacket->data() + pPacket->rpos(), web_pFragmentDatasRemain_);
					pPacket->read_skip(web_pFragmentDatasRemain_);
					web_pFragmentDatasRemain_ = 0;
					web_fragmentDatasFlag_ = FRAGMENT_DATA_MESSAGE_BODY;
					pTCPPacket_ = TCPPacket::ObjPool().createObject();
				}
				else
				{
					memcpy(&masks_[4 - web_pFragmentDatasRemain_], pPacket->data() + pPacket->rpos(), pPacket->totalSize());
					web_pFragmentDatasRemain_ -= pPacket->totalSize();
					pPacket->opfini();
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
				if(pPacket->totalSize() >= payloadSize_)
				{
					memcpy(pTCPPacket_->data() + pTCPPacket_->wpos(), pPacket->data() + pPacket->rpos(), payloadSize_);
					pTCPPacket_->wpos(pTCPPacket_->wpos() + payloadSize_);
					web_fragmentDatasFlag_ = FRAGMENT_DATA_BASIC_LENGTH;
					web_pFragmentDatasRemain_ = 2;
					pPacket->read_skip(payloadSize_);
					uint64 startSize = payloadTotalSize_ - payloadSize_;
					payloadSize_ = 0;

					for(uint64 i=0; i<pTCPPacket_->opsize(); i++)
					{
						pTCPPacket_->data()[i] = (pTCPPacket_->data()[i] ^ masks_[(i + startSize) % 4]);
					}

					web_fragmentDatasFlag_ = FRAGMENT_DATA_BASIC_LENGTH;
					web_pFragmentDatasRemain_ = 2;
					pRetTCPPacket = pTCPPacket_;
					pTCPPacket_ = NULL;

					if(pPacket->totalSize() == 0)
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
					memcpy(pTCPPacket_->data() + pTCPPacket_->wpos(), pPacket->data() + pPacket->rpos(), pPacket->totalSize());
					uint64 startSize = payloadTotalSize_ - payloadSize_;
					payloadSize_ -= pPacket->totalSize();
					pTCPPacket_->wpos(pTCPPacket_->wpos() + pPacket->totalSize());
					pPacket->opfini();
					pRetTCPPacket = pTCPPacket_;
					
					for(uint64 i=0; i<pTCPPacket_->opsize(); i++)
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
