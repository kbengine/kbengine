// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "packet_reader.h"
#include "network/channel.h"
#include "network/message_handler.h"
#include "network/network_stats.h"

namespace KBEngine { 
namespace Network
{


//-------------------------------------------------------------------------------------
PacketReader::PacketReader(Channel* pChannel):
	pFragmentDatas_(NULL),
	pFragmentDatasWpos_(0),
	pFragmentDatasRemain_(0),
	fragmentDatasFlag_(FRAGMENT_DATA_UNKNOW),
	pFragmentStream_(NULL),
	currMsgID_(0),
	currMsgLen_(0),
	pChannel_(pChannel)
{
}

//-------------------------------------------------------------------------------------
PacketReader::~PacketReader()
{
	reset();
	pChannel_ = NULL;
}

//-------------------------------------------------------------------------------------
void PacketReader::reset()
{
	fragmentDatasFlag_ = FRAGMENT_DATA_UNKNOW;
	pFragmentDatasWpos_ = 0;
	pFragmentDatasRemain_ = 0;
	currMsgID_ = 0;
	currMsgLen_ = 0;
	
	SAFE_RELEASE_ARRAY(pFragmentDatas_);
	MemoryStream::reclaimPoolObject(pFragmentStream_);
	pFragmentStream_ = NULL;
}

//-------------------------------------------------------------------------------------
void PacketReader::processMessages(KBEngine::Network::MessageHandlers* pMsgHandlers, Packet* pPacket)
{
	while(pPacket->length() > 0 || pFragmentStream_ != NULL)
	{
		if(fragmentDatasFlag_ == FRAGMENT_DATA_UNKNOW)
		{
			// 如果没有ID信息，先获取ID
			if(currMsgID_ == 0)
			{
				if(NETWORK_MESSAGE_ID_SIZE > 1 && pPacket->length() < NETWORK_MESSAGE_ID_SIZE)
				{
					writeFragmentMessage(FRAGMENT_DATA_MESSAGE_ID, pPacket, NETWORK_MESSAGE_ID_SIZE);
					break;
				}

				(*pPacket) >> currMsgID_;
				pPacket->messageID(currMsgID_);
			}

			Network::MessageHandler* pMsgHandler = pMsgHandlers->find(currMsgID_);

			if(pMsgHandler == NULL)
			{
				MemoryStream* pPacket1 = pFragmentStream_ != NULL ? pFragmentStream_ : pPacket;
				TRACE_MESSAGE_PACKET(true, pPacket1, pMsgHandler, pPacket1->length(), pChannel_->c_str(), false);
				
				// 用作调试时比对
				uint32 rpos = pPacket1->rpos();
				pPacket1->rpos(0);
				TRACE_MESSAGE_PACKET(true, pPacket1, pMsgHandler, pPacket1->length(), pChannel_->c_str(), false);
				pPacket1->rpos(rpos);

				ERROR_MSG(fmt::format("PacketReader::processMessages: not found msgID={}, msglen={}, from {}.\n",
					currMsgID_, pPacket1->length(), pChannel_->c_str()));

				currMsgID_ = 0;
				currMsgLen_ = 0;
				pChannel_->condemn("PacketReader::processMessages: not found msgID");
				break;
			}

			// 如果没有可操作的数据了则退出等待下一个包处理。
			// 可能是一个无参数数据包
			//if(pPacket->opsize() == 0)	
			//	break;
			
			// 如果长度信息没有获得，则等待获取长度信息
			if(currMsgLen_ == 0)
			{
				// 如果长度信息是可变的或者配置了永远包含长度信息选项时，从流中分析长度数据
				if(pMsgHandler->msgLen == NETWORK_VARIABLE_MESSAGE)
				{
					// 如果长度信息不完整，则等待下一个包处理
					if(pPacket->length() < NETWORK_MESSAGE_LENGTH_SIZE)
					{
						writeFragmentMessage(FRAGMENT_DATA_MESSAGE_LENGTH, pPacket, NETWORK_MESSAGE_LENGTH_SIZE);
						break;
					}
					else
					{
						// 此处获得了长度信息
						Network::MessageLength currlen;
						(*pPacket) >> currlen;
						currMsgLen_ = currlen;

						NetworkStats::getSingleton().trackMessage(NetworkStats::RECV, *pMsgHandler, 
							currMsgLen_ + NETWORK_MESSAGE_ID_SIZE + NETWORK_MESSAGE_LENGTH_SIZE);

						// 如果长度占满说明使用了扩展长度，我们还需要等待扩展长度信息
						if(currMsgLen_ == NETWORK_MESSAGE_MAX_SIZE)
						{
							if(pPacket->length() < NETWORK_MESSAGE_LENGTH1_SIZE)
							{
								// 如果长度信息不完整，则等待下一个包处理
								writeFragmentMessage(FRAGMENT_DATA_MESSAGE_LENGTH1, pPacket, NETWORK_MESSAGE_LENGTH1_SIZE);
								break;
							}
							else
							{
								// 此处获得了扩展长度信息
								(*pPacket) >> currMsgLen_;

								NetworkStats::getSingleton().trackMessage(NetworkStats::RECV, *pMsgHandler, 
									currMsgLen_ + NETWORK_MESSAGE_ID_SIZE + NETWORK_MESSAGE_LENGTH1_SIZE);
							}
						}
					}
				}
				else
				{
					currMsgLen_ = pMsgHandler->msgLen;

					NetworkStats::getSingleton().trackMessage(NetworkStats::RECV, *pMsgHandler, 
						currMsgLen_ + NETWORK_MESSAGE_LENGTH_SIZE);
				}
			}

			if(this->pChannel_->isExternal() && 
				g_componentType != BOTS_TYPE && 
				g_componentType != CLIENT_TYPE && 
				currMsgLen_ > NETWORK_MESSAGE_MAX_SIZE)
			{
				MemoryStream* pPacket1 = pFragmentStream_ != NULL ? pFragmentStream_ : pPacket;
				TRACE_MESSAGE_PACKET(true, pPacket1, pMsgHandler, pPacket1->length(), pChannel_->c_str(), false);

				// 用作调试时比对
				uint32 rpos = pPacket1->rpos();
				pPacket1->rpos(0);
				TRACE_MESSAGE_PACKET(true, pPacket1, pMsgHandler, pPacket1->length(), pChannel_->c_str(), false);
				pPacket1->rpos(rpos);

				WARNING_MSG(fmt::format("PacketReader::processMessages({0}): msglen exceeds the limit! msgID={1}, msglen=({2}:{3}), maxlen={5}, from {4}.\n", 
					pMsgHandler->name.c_str(), currMsgID_, currMsgLen_, pPacket1->length(), pChannel_->c_str(), NETWORK_MESSAGE_MAX_SIZE));

				currMsgLen_ = 0;
				pChannel_->condemn("PacketReader::processMessages: msglen exceeds the limit!");
				break;
			}

			if(pFragmentStream_ != NULL)
			{
				TRACE_MESSAGE_PACKET(true, pFragmentStream_, pMsgHandler, currMsgLen_, pChannel_->c_str(), false);
				pMsgHandler->handle(pChannel_, *pFragmentStream_);
				MemoryStream::reclaimPoolObject(pFragmentStream_);
				pFragmentStream_ = NULL;
			}
			else
			{
				if(pPacket->length() < currMsgLen_)
				{
					writeFragmentMessage(FRAGMENT_DATA_MESSAGE_BODY, pPacket, currMsgLen_);
					break;
				}

				// 临时设置有效读取位， 防止接口中溢出操作
				size_t wpos = pPacket->wpos();
				// size_t rpos = pPacket->rpos();
				size_t frpos = pPacket->rpos() + currMsgLen_;
				pPacket->wpos(frpos);

				TRACE_MESSAGE_PACKET(true, pPacket, pMsgHandler, currMsgLen_, pChannel_->c_str(), true);
				pMsgHandler->handle(pChannel_, *pPacket);

				// 如果handler没有处理完数据则输出一个警告
				if(currMsgLen_ > 0)
				{
					if(frpos != pPacket->rpos())
					{
						WARNING_MSG(fmt::format("PacketReader::processMessages({}): rpos({}) invalid, expect={}. msgID={}, msglen={}.\n",
							pMsgHandler->name.c_str(), pPacket->rpos(), frpos, currMsgID_, currMsgLen_));

						pPacket->rpos(frpos);
					}
				}

				pPacket->wpos(wpos);
			}

			currMsgID_ = 0;
			currMsgLen_ = 0;
		}
		else
		{
			mergeFragmentMessage(pPacket);
		}
	}
}

//-------------------------------------------------------------------------------------
void PacketReader::writeFragmentMessage(FragmentDataTypes fragmentDatasFlag, Packet* pPacket, uint32 datasize)
{
	KBE_ASSERT(pFragmentDatas_ == NULL);

	size_t opsize = pPacket->length();
	pFragmentDatasRemain_ = datasize - opsize;
	pFragmentDatas_ = new uint8[opsize + pFragmentDatasRemain_ + 1];

	fragmentDatasFlag_ = fragmentDatasFlag;
	pFragmentDatasWpos_ = opsize;

	if(pPacket->length() > 0)
	{
		memcpy(pFragmentDatas_, pPacket->data() + pPacket->rpos(), opsize);
		pPacket->done();
	}

	//DEBUG_MSG(fmt::format("PacketReader::writeFragmentMessage({}): channel[{:p}], fragmentDatasFlag={}, remainsize={}, currMsgID={}, currMsgLen={}.\n", 
	//	pChannel_->c_str(), (void*)pChannel_, fragmentDatasFlag, pFragmentDatasRemain_, currMsgID_, currMsgLen_));
}

//-------------------------------------------------------------------------------------
void PacketReader::mergeFragmentMessage(Packet* pPacket)
{
	size_t opsize = pPacket->length();
	if(opsize == 0)
		return;

	if(pPacket->length() >= pFragmentDatasRemain_)
	{
		memcpy(pFragmentDatas_ + pFragmentDatasWpos_, pPacket->data() + pPacket->rpos(), pFragmentDatasRemain_);
		pPacket->rpos(pPacket->rpos() + pFragmentDatasRemain_);

		KBE_ASSERT(pFragmentStream_ == NULL);

		switch(fragmentDatasFlag_)
		{
		case FRAGMENT_DATA_MESSAGE_ID:			// 消息ID信息不全
			memcpy(&currMsgID_, pFragmentDatas_, NETWORK_MESSAGE_ID_SIZE);
			break;

		case FRAGMENT_DATA_MESSAGE_LENGTH:		// 消息长度信息不全
			memcpy(&currMsgLen_, pFragmentDatas_, NETWORK_MESSAGE_LENGTH_SIZE);
			break;

		case FRAGMENT_DATA_MESSAGE_LENGTH1:		// 消息长度信息不全
			memcpy(&currMsgLen_, pFragmentDatas_, NETWORK_MESSAGE_LENGTH1_SIZE);
			break;

		case FRAGMENT_DATA_MESSAGE_BODY:		// 消息内容信息不全
			pFragmentStream_ = MemoryStream::createPoolObject(OBJECTPOOL_POINT);
			pFragmentStream_->append(pFragmentDatas_, currMsgLen_);
			break;

		default:
			break;
		};

		//DEBUG_MSG(fmt::format("PacketReader::mergeFragmentMessage({}): channel[{:p}], fragmentDatasFlag={}, currMsgID={}, currMsgLen={}, completed!\n", 
		//	pChannel_->c_str(), (void*)pChannel_, fragmentDatasFlag_, currMsgID_, currMsgLen_));

		fragmentDatasFlag_ = FRAGMENT_DATA_UNKNOW;
		pFragmentDatasRemain_ = 0;
		SAFE_RELEASE_ARRAY(pFragmentDatas_);
	}
	else
	{
		memcpy(pFragmentDatas_ + pFragmentDatasWpos_, pPacket->data(), opsize);
		pFragmentDatasRemain_ -= opsize;
		pFragmentDatasWpos_ += opsize;
		pPacket->rpos(pPacket->rpos() + opsize);

		//DEBUG_MSG(fmt::format("PacketReader::mergeFragmentMessage({}): channel[{:p}], fragmentDatasFlag={}, remainsize={}, currMsgID={}, currMsgLen={}.\n",
		//	pChannel_->c_str(), (void*)pChannel_, fragmentDatasFlag_, pFragmentDatasRemain_, currMsgID_, currMsgLen_));
	}	
}

//-------------------------------------------------------------------------------------
} 
}
