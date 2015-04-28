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
#include "websocket_protocol.h"

#include "network/bundle.h"
#include "network/channel.h"
#include "network/tcp_packet.h"
#include "network/network_interface.h"
#include "network/packet_receiver.h"

namespace KBEngine { 
namespace Network
{

//-------------------------------------------------------------------------------------
WebSocketPacketFilter::WebSocketPacketFilter(Channel* pChannel):
	pFragmentDatasRemain_(0),
	fragmentDatasFlag_(FRAGMENT_MESSAGE_HREAD),
	msg_opcode_(0),
	msg_fin_(0),
	msg_masked_(0),
	msg_mask_(0),
	msg_length_field_(0),
	msg_payload_length_(0),
	msg_frameType_(websocket::WebSocketProtocol::ERROR_FRAME),
	pChannel_(pChannel),
	pTCPPacket_(NULL)
{
}

//-------------------------------------------------------------------------------------
WebSocketPacketFilter::~WebSocketPacketFilter()
{
	reset();
}

//-------------------------------------------------------------------------------------
void WebSocketPacketFilter::reset()
{
	msg_opcode_ = 0;
	msg_fin_ = 0;
	msg_masked_ = 0;
	msg_mask_ = 0;
	msg_length_field_ = 0;
	msg_payload_length_ = 0;
	pFragmentDatasRemain_ = 0;
	fragmentDatasFlag_ = FRAGMENT_MESSAGE_HREAD;

	TCPPacket::ObjPool().reclaimObject(pTCPPacket_);
	pTCPPacket_ = NULL;
}

//-------------------------------------------------------------------------------------
Reason WebSocketPacketFilter::send(Channel * pChannel, PacketSender& sender, Packet * pPacket)
{
	if(pPacket->encrypted())
		return PacketFilter::send(pChannel, sender, pPacket);

	Bundle* pBundle = pPacket->pBundle();
	TCPPacket* pRetTCPPacket = TCPPacket::ObjPool().createObject();
	websocket::WebSocketProtocol::FrameType frameType = websocket::WebSocketProtocol::BINARY_FRAME;

	if(pBundle && pBundle->packets().size() > 1)
	{
		bool isEnd = pBundle->packets().back() == pPacket;
		bool isBegin = pBundle->packets().front() == pPacket;

		if(!isEnd && !isBegin)
		{
			frameType = websocket::WebSocketProtocol::NEXT_FRAME;
		}
		else
		{
			if(!isEnd)
			{
				frameType = websocket::WebSocketProtocol::INCOMPLETE_BINARY_FRAME;
			}
			else
			{
				frameType = websocket::WebSocketProtocol::END_FRAME;
			}
		}
	}

	websocket::WebSocketProtocol::makeFrame(frameType, pPacket, pRetTCPPacket);

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

	pPacket->encrypted(true);
	return PacketFilter::send(pChannel, sender, pPacket);
}

//-------------------------------------------------------------------------------------
Reason WebSocketPacketFilter::recv(Channel * pChannel, PacketReceiver & receiver, Packet * pPacket)
{
	while(pPacket->length() > 0)
	{
		if(fragmentDatasFlag_ == FRAGMENT_MESSAGE_HREAD)
		{
			if(pFragmentDatasRemain_ == 0)
			{
				KBE_ASSERT(pTCPPacket_ == NULL);

				size_t rpos = pPacket->rpos();

				reset();

				// 如果没有创建过缓存，先尝试直接解析包头，如果信息足够成功解析则继续到下一步
				pFragmentDatasRemain_ = websocket::WebSocketProtocol::getFrame(pPacket, msg_opcode_, msg_fin_, msg_masked_, 
					msg_mask_, msg_length_field_, msg_payload_length_, msg_frameType_);

				if(pFragmentDatasRemain_ > 0)
				{
					pPacket->rpos(rpos);
					pTCPPacket_ = TCPPacket::ObjPool().createObject();
					pTCPPacket_->append(*(static_cast<MemoryStream*>(pPacket)));
					pPacket->done();
				}
				else
				{
					fragmentDatasFlag_ = FRAGMENT_MESSAGE_DATAS;
					pFragmentDatasRemain_ = (int32)msg_payload_length_;
				}
			}
			else
			{
				KBE_ASSERT(pTCPPacket_ != NULL);

				// 长度如果大于剩余读取长度，那么可以开始解析了
				// 否则将包内存继续缓存
				if((int32)pPacket->length() >= pFragmentDatasRemain_)
				{
					pFragmentDatasRemain_ = websocket::WebSocketProtocol::getFrame(pTCPPacket_, msg_opcode_, msg_fin_, msg_masked_, 
						msg_mask_, msg_length_field_, msg_payload_length_, msg_frameType_);

					KBE_ASSERT(pFragmentDatasRemain_ == 0);

					// frame解析完毕，将对象回收
					TCPPacket::ObjPool().reclaimObject(pTCPPacket_);
					pTCPPacket_ = NULL;

					// 是否有数据携带？如果没有则不进入data解析
					if(msg_payload_length_ > 0)
					{
						fragmentDatasFlag_ = FRAGMENT_MESSAGE_DATAS;
						pFragmentDatasRemain_ = (int32)msg_payload_length_;
					}
				}
				else
				{
					pTCPPacket_->append(*(static_cast<MemoryStream*>(pPacket)));
					pFragmentDatasRemain_ -= pPacket->length();

					pPacket->done();
				}
			}

			if(websocket::WebSocketProtocol::ERROR_FRAME == msg_frameType_)
			{
				ERROR_MSG(fmt::format("WebSocketPacketReader::recv: frame is error! addr={}!\n",
					pChannel_->c_str()));

				this->pChannel_->condemn();
				reset();

				TCPPacket::ObjPool().reclaimObject(static_cast<TCPPacket*>(pPacket));
				return REASON_WEBSOCKET_ERROR;
			}
			else if(msg_frameType_ == websocket::WebSocketProtocol::TEXT_FRAME || 
					msg_frameType_ == websocket::WebSocketProtocol::INCOMPLETE_TEXT_FRAME ||
					msg_frameType_ == websocket::WebSocketProtocol::PING_FRAME ||
					msg_frameType_ == websocket::WebSocketProtocol::PONG_FRAME)
			{
				ERROR_MSG(fmt::format("WebSocketPacketReader::recv: Does not support FRAME_TYPE()! addr={}!\n",
					(int)msg_frameType_, pChannel_->c_str()));

				this->pChannel_->condemn();
				reset();

				TCPPacket::ObjPool().reclaimObject(static_cast<TCPPacket*>(pPacket));
				return REASON_WEBSOCKET_ERROR;
			}
			else if(msg_frameType_ == websocket::WebSocketProtocol::CLOSE_FRAME)
			{
				this->pChannel_->condemn();
				reset();

				TCPPacket::ObjPool().reclaimObject(static_cast<TCPPacket*>(pPacket));
				return REASON_SUCCESS;
			}
			else if(msg_frameType_ == websocket::WebSocketProtocol::INCOMPLETE_FRAME)
			{
				// 继续等待后续内容到达
			}
		}
		else
		{
			KBE_ASSERT(pFragmentDatasRemain_ > 0);

			if(pTCPPacket_ == NULL)
				pTCPPacket_ = TCPPacket::ObjPool().createObject();

			if(pFragmentDatasRemain_ <= (int32)pPacket->length())
			{
				pTCPPacket_->append(pPacket->data() + pPacket->rpos(), pFragmentDatasRemain_);
				pPacket->read_skip((size_t)pFragmentDatasRemain_);
				pFragmentDatasRemain_ = 0;
			}
			else
			{
				pTCPPacket_->append(*(static_cast<MemoryStream*>(pPacket)));
				pFragmentDatasRemain_ -= pPacket->length();
				pPacket->done();
			}

			if(!websocket::WebSocketProtocol::decodingDatas(pTCPPacket_, msg_masked_, msg_mask_))
			{
				ERROR_MSG(fmt::format("WebSocketPacketReader::recv: decoding-frame is error! addr={}!\n",
					pChannel_->c_str()));

				this->pChannel_->condemn();
				reset();

				TCPPacket::ObjPool().reclaimObject(static_cast<TCPPacket*>(pPacket));
				return REASON_WEBSOCKET_ERROR;
			}

			Reason reason = PacketFilter::recv(pChannel, receiver, pTCPPacket_);

			// pTCPPacket_不需要在这里回收了
			pTCPPacket_ = NULL;

			if(pFragmentDatasRemain_ == 0)
				reset();

			if(reason != REASON_SUCCESS)
				return reason;
		}
	}

	TCPPacket::ObjPool().reclaimObject(static_cast<TCPPacket*>(pPacket));
	return REASON_SUCCESS;
}

//-------------------------------------------------------------------------------------
} 
}
