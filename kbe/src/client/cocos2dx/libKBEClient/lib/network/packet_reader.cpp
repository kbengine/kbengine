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

#include "packet_reader.hpp"
#include "network/channel.hpp"
#include "network/message_handler.hpp"
#include "network/mercurystats.hpp"

namespace KBEngine { 
namespace Mercury
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
	MemoryStream::ObjPool().reclaimObject(pFragmentStream_);
	pFragmentStream_ = NULL;
}

//-------------------------------------------------------------------------------------
void PacketReader::processMessages(KBEngine::Mercury::MessageHandlers* pMsgHandlers, Packet* pPacket)
{
	while(pPacket->totalSize() > 0 || pFragmentStream_ != NULL)
	{
		if(fragmentDatasFlag_ == FRAGMENT_DATA_UNKNOW)
		{
			if(currMsgID_ == 0)
			{
				if(MERCURY_MESSAGE_ID_SIZE > 1 && pPacket->opsize() < MERCURY_MESSAGE_ID_SIZE)
				{
					writeFragmentMessage(FRAGMENT_DATA_MESSAGE_ID, pPacket, MERCURY_MESSAGE_ID_SIZE);
					break;
				}

				(*pPacket) >> currMsgID_;
				pPacket->messageID(currMsgID_);
			}

			Mercury::MessageHandler* pMsgHandler = pMsgHandlers->find(currMsgID_);

			if(pMsgHandler == NULL)
			{
				MemoryStream* pPacket1 = pFragmentStream_ != NULL ? pFragmentStream_ : pPacket;
				TRACE_BUNDLE_DATA(true, pPacket1, pMsgHandler, pPacket1->opsize(), pChannel_->c_str());
				
				// 用作调试时比对
				uint32 rpos = pPacket1->rpos();
				pPacket1->rpos(0);
				TRACE_BUNDLE_DATA(true, pPacket1, pMsgHandler, pPacket1->opsize(), pChannel_->c_str());
				pPacket1->rpos(rpos);

//				WARNING_MSG(boost::format("PacketReader::processMessages: invalide msgID=%1%, msglen=%2%, from %3%.\n") %
//					currMsgID_ % pPacket1->opsize() % pChannel_->c_str());

				currMsgID_ = 0;
				currMsgLen_ = 0;
				pChannel_->condemn();
				break;
			}

			// 如果没有可操作的数据了则退出等待下一个包处理。
			//if(pPacket->opsize() == 0)	// 可能是一个无参数数据包
			//	break;
			
			if(currMsgLen_ == 0)
			{
				if(pMsgHandler->msgLen == MERCURY_VARIABLE_MESSAGE || g_packetAlwaysContainLength)
				{
					// 如果长度信息不完整， 则等待下一个包处理
					if(pPacket->opsize() < MERCURY_MESSAGE_LENGTH_SIZE)
					{
						writeFragmentMessage(FRAGMENT_DATA_MESSAGE_LENGTH, pPacket, MERCURY_MESSAGE_LENGTH_SIZE);
						break;
					}
					else
					{
						(*pPacket) >> currMsgLen_;
						MercuryStats::getSingleton().trackMessage(MercuryStats::RECV, *pMsgHandler, 
							currMsgLen_ + MERCURY_MESSAGE_ID_SIZE + MERCURY_MESSAGE_LENGTH_SIZE);
					}
				}
				else
				{
					currMsgLen_ = pMsgHandler->msgLen;
					MercuryStats::getSingleton().trackMessage(MercuryStats::RECV, *pMsgHandler, 
						currMsgLen_ + MERCURY_MESSAGE_LENGTH_SIZE);
				}
			}
			
			if(currMsgLen_ > pMsgHandler->msglenMax())
			{
				MemoryStream* pPacket1 = pFragmentStream_ != NULL ? pFragmentStream_ : pPacket;
				TRACE_BUNDLE_DATA(true, pPacket1, pMsgHandler, pPacket1->opsize(), pChannel_->c_str());

				// 用作调试时比对
				uint32 rpos = pPacket1->rpos();
				pPacket1->rpos(0);
				TRACE_BUNDLE_DATA(true, pPacket1, pMsgHandler, pPacket1->opsize(), pChannel_->c_str());
				pPacket1->rpos(rpos);

//				WARNING_MSG(boost::format("PacketReader::processMessages(%1%): msglen is error! msgID=%2%, msglen=(%3%:%4%), from %5%.\n") % 
//					pMsgHandler->name.c_str() % currMsgID_ % currMsgLen_ % pPacket1->opsize() % pChannel_->c_str());

				currMsgLen_ = 0;
				pChannel_->condemn();
				break;
			}

			if(pFragmentStream_ != NULL)
			{
				TRACE_BUNDLE_DATA(true, pFragmentStream_, pMsgHandler, currMsgLen_, pChannel_->c_str());
				pMsgHandler->handle(pChannel_, *pFragmentStream_);
				MemoryStream::ObjPool().reclaimObject(pFragmentStream_);
				pFragmentStream_ = NULL;
			}
			else
			{
				if(pPacket->opsize() < currMsgLen_)
				{
					writeFragmentMessage(FRAGMENT_DATA_MESSAGE_BODY, pPacket, currMsgLen_);
					break;
				}

				// 临时设置有效读取位， 防止接口中溢出操作
				size_t wpos = pPacket->wpos();
				// size_t rpos = pPacket->rpos();
				size_t frpos = pPacket->rpos() + currMsgLen_;
				pPacket->wpos(frpos);

				TRACE_BUNDLE_DATA(true, pPacket, pMsgHandler, currMsgLen_, pChannel_->c_str());
				pMsgHandler->handle(pChannel_, *pPacket);

				// 如果handler没有处理完数据则输出一个警告
				if(currMsgLen_ > 0)
				{
					if(frpos != pPacket->rpos())
					{
//						WARNING_MSG(boost::format("PacketReader::processMessages(%s): rpos(%d) invalid, expect=%d. msgID=%d, msglen=%d.\n") %
//							pMsgHandler->name.c_str() % pPacket->rpos() % frpos % currMsgID_ % currMsgLen_);

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

	size_t opsize = pPacket->opsize();
	pFragmentDatasRemain_ = datasize - opsize;
	pFragmentDatas_ = new uint8[opsize + pFragmentDatasRemain_ + 1];

	fragmentDatasFlag_ = fragmentDatasFlag;
	pFragmentDatasWpos_ = opsize;

	if(pPacket->opsize() > 0)
	{
		memcpy(pFragmentDatas_, pPacket->data() + pPacket->rpos(), opsize);
		pPacket->opfini();
	}

//	DEBUG_MSG(boost::format("PacketReader::writeFragmentMessage(%1%): channel[%2%], fragmentDatasFlag=%3%, remainsize=%4%.\n") % 
//		pChannel_->c_str() % pChannel_ % fragmentDatasFlag % pFragmentDatasRemain_);
}

//-------------------------------------------------------------------------------------
void PacketReader::mergeFragmentMessage(Packet* pPacket)
{
	size_t opsize = pPacket->opsize();
	if(opsize == 0)
		return;

	if(pPacket->opsize() >= pFragmentDatasRemain_)
	{
		pPacket->rpos(pPacket->rpos() + pFragmentDatasRemain_);
		memcpy(pFragmentDatas_ + pFragmentDatasWpos_, pPacket->data(), pFragmentDatasRemain_);
		
		KBE_ASSERT(pFragmentStream_ == NULL);

		switch(fragmentDatasFlag_)
		{
		case FRAGMENT_DATA_MESSAGE_ID:			// 消息ID信息不全
			memcpy(&currMsgID_, pFragmentDatas_, MERCURY_MESSAGE_ID_SIZE);
			break;

		case FRAGMENT_DATA_MESSAGE_LENGTH:		// 消息长度信息不全
			memcpy(&currMsgLen_, pFragmentDatas_, MERCURY_MESSAGE_LENGTH_SIZE);
			break;

		case FRAGMENT_DATA_MESSAGE_BODY:		// 消息内容信息不全
			pFragmentStream_ = MemoryStream::ObjPool().createObject();
			pFragmentStream_->data_resize(currMsgLen_);
			pFragmentStream_->wpos(currMsgLen_);
			memcpy(pFragmentStream_->data(), pFragmentDatas_, currMsgLen_);
			break;

		default:
			break;
		};

		fragmentDatasFlag_ = FRAGMENT_DATA_UNKNOW;
		pFragmentDatasRemain_ = 0;
		SAFE_RELEASE_ARRAY(pFragmentDatas_);
//		DEBUG_MSG(boost::format("PacketReader::mergeFragmentMessage(%1%): channel[%2%], completed!\n") % 
//			pChannel_->c_str() % pChannel_);
	}
	else
	{
		memcpy(pFragmentDatas_ + pFragmentDatasWpos_, pPacket->data(), opsize);
		pFragmentDatasRemain_ -= opsize;
		pFragmentDatasWpos_ += opsize;
		pPacket->rpos(pPacket->rpos() + opsize);

//		DEBUG_MSG(boost::format("PacketReader::writeFragmentMessage(%1%): channel[%2%], fragmentDatasFlag=%3%, remainsize=%4%.\n") %
//			pChannel_->c_str() % pChannel_ % fragmentDatasFlag_ % pFragmentDatasRemain_);
	}
}

//-------------------------------------------------------------------------------------
} 
}
