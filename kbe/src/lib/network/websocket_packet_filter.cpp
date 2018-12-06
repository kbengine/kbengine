// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


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

	TCPPacket::reclaimPoolObject(pTCPPacket_);
	pTCPPacket_ = NULL;
}

//-------------------------------------------------------------------------------------
Reason WebSocketPacketFilter::send(Channel * pChannel, PacketSender& sender, Packet * pPacket, int userarg)
{
	if(pPacket->encrypted())
		return PacketFilter::send(pChannel, sender, pPacket, userarg);

	Bundle* pBundle = pPacket->pBundle();
	TCPPacket* pRetTCPPacket = TCPPacket::createPoolObject(OBJECTPOOL_POINT);
	websocket::WebSocketProtocol::FrameType frameType = websocket::WebSocketProtocol::BINARY_FRAME;

	if (pBundle)
	{
		Bundle::Packets& packs = pBundle->packets();

		if (packs.size() > 1)
		{
			bool isEnd = packs.back() == pPacket;
			bool isBegin = packs.front() == pPacket;

			if (!isEnd && !isBegin)
			{
				frameType = websocket::WebSocketProtocol::NEXT_FRAME;
			}
			else
			{
				if (!isEnd)
				{
					frameType = websocket::WebSocketProtocol::INCOMPLETE_BINARY_FRAME;
				}
				else
				{
					frameType = websocket::WebSocketProtocol::END_FRAME;
				}
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
	TCPPacket::reclaimPoolObject(pRetTCPPacket);

	pPacket->encrypted(true);
	return PacketFilter::send(pChannel, sender, pPacket, userarg);
}

//-------------------------------------------------------------------------------------
Reason WebSocketPacketFilter::recv(Channel * pChannel, PacketReceiver & receiver, Packet * pPacket)
{
	while (pPacket->length() > 0)
	{
		if (fragmentDatasFlag_ == FRAGMENT_MESSAGE_HREAD)
		{
			if (pFragmentDatasRemain_ == 0)
			{
				KBE_ASSERT(pTCPPacket_ == NULL);

				size_t rpos = pPacket->rpos();

				reset();

				// 如果没有创建过缓存，先尝试直接解析包头，如果信息足够成功解析则继续到下一步
				pFragmentDatasRemain_ = websocket::WebSocketProtocol::getFrame(pPacket, msg_opcode_, msg_fin_, msg_masked_,
					msg_mask_, msg_length_field_, msg_payload_length_, msg_frameType_);

				if (pFragmentDatasRemain_ > 0)
				{
					pPacket->rpos(rpos);
					pTCPPacket_ = TCPPacket::createPoolObject(OBJECTPOOL_POINT);
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
				if ((int32)pPacket->length() >= pFragmentDatasRemain_)
				{
					size_t wpos = pPacket->wpos();
					size_t rpos = pPacket->rpos();

					pPacket->wpos(rpos + pFragmentDatasRemain_);

					// 首先将需要的数据添加到pTCPPacket_
					pTCPPacket_->append(*(static_cast<MemoryStream*>(pPacket)));

					// 将写位置还原回去
					pPacket->wpos(wpos);

					// 丢弃已经读取的数据
					pPacket->read_skip(pFragmentDatasRemain_);

					size_t buffer_rpos = pTCPPacket_->rpos();
					pFragmentDatasRemain_ = websocket::WebSocketProtocol::getFrame(pTCPPacket_, msg_opcode_, msg_fin_, msg_masked_,
						msg_mask_, msg_length_field_, msg_payload_length_, msg_frameType_);

					// 如果仍然大于0， 说明需要继续收包
					if (pFragmentDatasRemain_ > 0)
					{
						// 由于一次没有解析完， 我们回撤数据下一次再尝试解析
						pTCPPacket_->rpos(buffer_rpos);

						// 当前包如果还有数据并且大于等于我们需要的数据，则继续下一循环立即解析
						if ((int32)pPacket->length() >= pFragmentDatasRemain_)
							continue;
					}
					else
					{
						// frame解析完毕，将对象回收
						TCPPacket::reclaimPoolObject(pTCPPacket_);
						pTCPPacket_ = NULL;

						// 是否有数据携带？如果没有则不进入data解析
						if (msg_payload_length_ > 0)
						{
							fragmentDatasFlag_ = FRAGMENT_MESSAGE_DATAS;
							pFragmentDatasRemain_ = (int32)msg_payload_length_;
						}
					}
				}
				else
				{
					pTCPPacket_->append(*(static_cast<MemoryStream*>(pPacket)));
					pFragmentDatasRemain_ -= pPacket->length();

					pPacket->done();
				}
			}

			if (websocket::WebSocketProtocol::ERROR_FRAME == msg_frameType_)
			{
				ERROR_MSG(fmt::format("WebSocketPacketFilter::recv: frame error! addr={}!\n",
					pChannel_->c_str()));

				this->pChannel_->condemn("WebSocketPacketFilter::recv: frame error!");
				reset();

				TCPPacket::reclaimPoolObject(static_cast<TCPPacket*>(pPacket));
				return REASON_WEBSOCKET_ERROR;
			}
			else if (msg_frameType_ == websocket::WebSocketProtocol::TEXT_FRAME ||
				msg_frameType_ == websocket::WebSocketProtocol::INCOMPLETE_TEXT_FRAME ||
				msg_frameType_ == websocket::WebSocketProtocol::PONG_FRAME)
			{
				ERROR_MSG(fmt::format("WebSocketPacketFilter::recv: Does not support FRAME_TYPE({})! addr={}!\n",
					websocket::WebSocketProtocol::getFrameTypeName(msg_frameType_), pChannel_->c_str()));

				this->pChannel_->condemn("WebSocketPacketFilter::recv: Does not support FRAME_TYPE");
				reset();

				TCPPacket::reclaimPoolObject(static_cast<TCPPacket*>(pPacket));
				return REASON_WEBSOCKET_ERROR;
			}
			else if (msg_frameType_ == websocket::WebSocketProtocol::CLOSE_FRAME)
			{
				this->pChannel_->condemn("");
				reset();

				TCPPacket::reclaimPoolObject(static_cast<TCPPacket*>(pPacket));
				return REASON_SUCCESS;
			}
			else if (msg_frameType_ == websocket::WebSocketProtocol::INCOMPLETE_FRAME)
			{
				// 继续等待后续内容到达
			}
			else if (msg_frameType_ == websocket::WebSocketProtocol::PING_FRAME)
			{
				if (pFragmentDatasRemain_ <= 0)
				{
					Reason reason = onPing(pChannel, pPacket);
					if (reason != REASON_SUCCESS)
					{
						reset();

						TCPPacket::reclaimPoolObject(static_cast<TCPPacket*>(pPacket));
						return reason;
					}
				}

				continue;
			}
		}
		else
		{
			if (pFragmentDatasRemain_ <= 0)
			{
				ERROR_MSG(fmt::format("WebSocketPacketFilter::recv: pFragmentDatasRemain_ <= 0! addr={}!\n",
					pChannel_->c_str()));

				this->pChannel_->condemn("WebSocketPacketFilter::recv: pFragmentDatasRemain_ <= 0!");
				reset();

				TCPPacket::reclaimPoolObject(static_cast<TCPPacket*>(pPacket));
				return REASON_WEBSOCKET_ERROR;
			}

			if (pTCPPacket_ == NULL)
				pTCPPacket_ = TCPPacket::createPoolObject(OBJECTPOOL_POINT);

			if (pFragmentDatasRemain_ <= (int32)pPacket->length())
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

			Reason reason = REASON_SUCCESS;

			if (msg_frameType_ == websocket::WebSocketProtocol::PING_FRAME)
			{
				// 继续等剩余的内容到来为止
				if (pFragmentDatasRemain_ > 0)
					continue;

				if (!websocket::WebSocketProtocol::decodingDatas(pTCPPacket_, msg_masked_, msg_mask_))
				{
					ERROR_MSG(fmt::format("WebSocketPacketFilter::recv: decoding-frame error! addr={}!\n",
						pChannel_->c_str()));

					this->pChannel_->condemn("WebSocketPacketFilter::recv: decoding-frame error!");
					reset();

					TCPPacket::reclaimPoolObject(static_cast<TCPPacket*>(pPacket));
					return REASON_WEBSOCKET_ERROR;
				}

				reason = onPing(pChannel, pTCPPacket_);
			}
			else
			{
				if (!websocket::WebSocketProtocol::decodingDatas(pTCPPacket_, msg_masked_, msg_mask_))
				{
					ERROR_MSG(fmt::format("WebSocketPacketFilter::recv: decoding-frame error! addr={}!\n",
						pChannel_->c_str()));

					this->pChannel_->condemn("WebSocketPacketFilter::recv: decoding-frame error!");
					reset();

					TCPPacket::reclaimPoolObject(static_cast<TCPPacket*>(pPacket));
					return REASON_WEBSOCKET_ERROR;
				}

				reason = PacketFilter::recv(pChannel, receiver, pTCPPacket_);
				KBE_ASSERT(reason == REASON_SUCCESS);

				// pTCPPacket_不需要在这里回收了
				pTCPPacket_ = NULL;
			}

			if (pFragmentDatasRemain_ == 0)
				reset();

			if (reason != REASON_SUCCESS)
			{
				TCPPacket::reclaimPoolObject(static_cast<TCPPacket*>(pPacket));
				reset();
				return reason;
			}
		}
	}

	TCPPacket::reclaimPoolObject(static_cast<TCPPacket*>(pPacket));
	return REASON_SUCCESS;
}

//-------------------------------------------------------------------------------------
Reason WebSocketPacketFilter::onPing(Channel * pChannel, Packet* pPacket)
{
	KBE_ASSERT(pFragmentDatasRemain_ == 0);

	TCPPacket* pPongPacket = TCPPacket::createPoolObject(OBJECTPOOL_POINT);
	websocket::WebSocketProtocol::makeFrame(websocket::WebSocketProtocol::PONG_FRAME, pPacket, pPongPacket);

	if (msg_payload_length_ > 0)
	{
		pPongPacket->append(pPacket->data() + pPacket->rpos(), msg_payload_length_);
		pPacket->read_skip((size_t)msg_payload_length_);
	}

	int sendSize = pPongPacket->length();

	while (sendSize > 0)
	{
		int ret = pChannel->pEndPoint()->send(pPongPacket->data() + (pPongPacket->length() - sendSize), sendSize);
		if (ret <= 0)
		{
			ERROR_MSG(fmt::format("WebSocketPacketFilter::recv: send({}) pong-frame error! addr={}, sendSize={}\n",
				ret, pChannel_->c_str(), sendSize));

			break;
		}

		sendSize -= ret;
	}

	TCPPacket::reclaimPoolObject(pPongPacket);

	pFragmentDatasRemain_ = 0;
	fragmentDatasFlag_ = FRAGMENT_MESSAGE_HREAD;
	return REASON_SUCCESS;
}

//-------------------------------------------------------------------------------------
} 
}
