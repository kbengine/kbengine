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
				
				// ��������ʱ�ȶ�
				uint32 rpos = pPacket1->rpos();
				pPacket1->rpos(0);
				TRACE_BUNDLE_DATA(true, pPacket1, pMsgHandler, pPacket1->opsize(), pChannel_->c_str());
				pPacket1->rpos(rpos);

				WARNING_MSG(fmt::format("PacketReader::processMessages: invalide msgID={}, msglen={}, from {}.\n",
					currMsgID_, pPacket1->opsize(), pChannel_->c_str()));

				currMsgID_ = 0;
				currMsgLen_ = 0;
				pChannel_->condemn();
				break;
			}

			// ���û�пɲ��������������˳��ȴ���һ��������
			//if(pPacket->opsize() == 0)	// ������һ���޲������ݰ�
			//	break;
			
			if(currMsgLen_ == 0)
			{
				if(pMsgHandler->msgLen == MERCURY_VARIABLE_MESSAGE || g_packetAlwaysContainLength)
				{
					// ���������Ϣ�������� ��ȴ���һ��������
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

				// ��������ʱ�ȶ�
				uint32 rpos = pPacket1->rpos();
				pPacket1->rpos(0);
				TRACE_BUNDLE_DATA(true, pPacket1, pMsgHandler, pPacket1->opsize(), pChannel_->c_str());
				pPacket1->rpos(rpos);

				WARNING_MSG(fmt::format("PacketReader::processMessages({0}): msglen exceeds the limit! msgID={1}, msglen=({2}:{3}), maxlen={5}, from {4}.\n", 
					pMsgHandler->name.c_str(), currMsgID_, currMsgLen_, pPacket1->opsize(), pChannel_->c_str(), pMsgHandler->msglenMax()));

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

				// ��ʱ������Ч��ȡλ�� ��ֹ�ӿ����������
				size_t wpos = pPacket->wpos();
				// size_t rpos = pPacket->rpos();
				size_t frpos = pPacket->rpos() + currMsgLen_;
				pPacket->wpos(frpos);

				TRACE_BUNDLE_DATA(true, pPacket, pMsgHandler, currMsgLen_, pChannel_->c_str());
				pMsgHandler->handle(pChannel_, *pPacket);

				// ���handlerû�д��������������һ������
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

	DEBUG_MSG(fmt::format("PacketReader::writeFragmentMessage({}): channel[{:p}], fragmentDatasFlag={}, remainsize={}, currMsgID={}, currMsgLen={}.\n", 
		pChannel_->c_str(), (void*)pChannel_, fragmentDatasFlag, pFragmentDatasRemain_, currMsgID_, currMsgLen_));
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
		case FRAGMENT_DATA_MESSAGE_ID:			// ��ϢID��Ϣ��ȫ
			memcpy(&currMsgID_, pFragmentDatas_, MERCURY_MESSAGE_ID_SIZE);
			break;

		case FRAGMENT_DATA_MESSAGE_LENGTH:		// ��Ϣ������Ϣ��ȫ
			memcpy(&currMsgLen_, pFragmentDatas_, MERCURY_MESSAGE_LENGTH_SIZE);
			break;

		case FRAGMENT_DATA_MESSAGE_BODY:		// ��Ϣ������Ϣ��ȫ
			pFragmentStream_ = MemoryStream::ObjPool().createObject();
			pFragmentStream_->data_resize(currMsgLen_);
			pFragmentStream_->wpos(currMsgLen_);
			memcpy(pFragmentStream_->data(), pFragmentDatas_, currMsgLen_);
			break;

		default:
			break;
		};

		DEBUG_MSG(fmt::format("PacketReader::mergeFragmentMessage({}): channel[{:p}], fragmentDatasFlag={}, currMsgID={}, currMsgLen={}, completed!\n", 
			pChannel_->c_str(), (void*)pChannel_, fragmentDatasFlag_, currMsgID_, currMsgLen_));

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

		DEBUG_MSG(fmt::format("PacketReader::mergeFragmentMessage({}): channel[{:p}], fragmentDatasFlag={}, remainsize={}, currMsgID={}, currMsgLen={}.\n",
			pChannel_->c_str(), (void*)pChannel_, fragmentDatasFlag_, pFragmentDatasRemain_, currMsgID_, currMsgLen_));
	}	
}

//-------------------------------------------------------------------------------------
} 
}
