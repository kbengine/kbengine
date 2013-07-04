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

#include "html5_packet_reader.hpp"
#include "network/channel.hpp"
#include "network/message_handler.hpp"
#include "network/mercurystats.hpp"

namespace KBEngine { 
namespace Mercury
{


//-------------------------------------------------------------------------------------
HTML5PacketReader::HTML5PacketReader(Channel* pChannel):
	PacketReader(pChannel),
	web_pFragmentDatasRemain_(2),
	web_fragmentDatasFlag_(FRAGMENT_DATA_BASIC_LENGTH),
	basicSize_(0),
	payloadSize_(0),
	data_xy_()
{
}

//-------------------------------------------------------------------------------------
HTML5PacketReader::~HTML5PacketReader()
{
}

//-------------------------------------------------------------------------------------
void HTML5PacketReader::reset()
{
	PacketReader::reset();
	web_fragmentDatasFlag_ = FRAGMENT_DATA_BASIC_LENGTH;
	web_pFragmentDatasRemain_ = 2;
	payloadSize_ = 0;
	basicSize_= 0;
	data_xy_.first = 0;
	data_xy_.second = 0;
}

//-------------------------------------------------------------------------------------
void HTML5PacketReader::processMessages(KBEngine::Mercury::MessageHandlers* pMsgHandlers, Packet* pPacket)
{
	/*
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
						return;
					}
					else
					{
						(*pPacket) >> basicSize_;
					}
				}

				(*pPacket) >> basicSize_;
				basicSize_ &= 0x7f;

				switch(basicSize_)
				{
				case 126:
					data_xy_.first = 2;
					data_xy_.second = 7;
					// web_pFragmentDatasRemain_ = 6;
					web_pFragmentDatasRemain_ = 2;
					web_fragmentDatasFlag_ = FRAGMENT_DATA_PAYLOAD_LENGTH;
					break;
				case 127:
					data_xy_.first = 8;
					data_xy_.second = 13;
					// web_pFragmentDatasRemain_ = 12;
					web_pFragmentDatasRemain_ = 2;
					web_fragmentDatasFlag_ = FRAGMENT_DATA_PAYLOAD_LENGTH;
					break;
				default:
					if(basicSize > 127)
					{
						this->pChannel_->condemn();
						ERROR_MSG(boost::format("HTML5PacketReader::processMessages: basicSize(%1%) is error! addr=%2%!\n") % 
							basicSize % pChannel_->c_str());
						return;
					}

					data_xy_.first = 0;
					data_xy_.second = 4;
					web_pFragmentDatasRemain_ = 4;
					payloadSize_ = basicSize_;
					web_fragmentDatasFlag_ = FRAGMENT_DATA_PAYLOAD_MASKS;
					break;
				};
			}

			if(web_fragmentDatasFlag_ == FRAGMENT_DATA_PAYLOAD_LENGTH)
			{
				if(pPacket->totalSize() >= web_pFragmentDatasRemain_)
				{
					memcpy(&masks_[2 - web_pFragmentDatasRemain_], pPacket->data() + pPacket->rpos(), web_pFragmentDatasRemain_);
					pPacket->read_skip(web_pFragmentDatasRemain_);
					web_pFragmentDatasRemain_ = 0;
					web_fragmentDatasFlag_ = FRAGMENT_DATA_PAYLOAD_MASKS;

					switch(basicSize_)
					{
					case 126:
						web_pFragmentDatasRemain_ = 6;
						break;
					case 127:
						web_pFragmentDatasRemain_ = 12;
						break;
					default:
						payloadSize_ = basicSize_;
						break;
					};

				}
				else
				{
					memcpy(&masks_[2 - web_pFragmentDatasRemain_], pPacket->data() + pPacket->rpos(), web_pFragmentDatasRemain_ - pPacket->totalSize());
					web_pFragmentDatasRemain_ -= pPacket->totalSize();
					pPacket->opfini();
					return;
				}
			}

			if(web_fragmentDatasFlag_ == FRAGMENT_DATA_PAYLOAD_MASKS)
			{
				if(pPacket->totalSize() >= web_pFragmentDatasRemain_)
				{
					memcpy(&masks_[data_xy_.second - 1 - web_pFragmentDatasRemain_], pPacket->data() + pPacket->rpos(), web_pFragmentDatasRemain_);
					pPacket->read_skip(web_pFragmentDatasRemain_);
					web_pFragmentDatasRemain_ = 0;
					web_fragmentDatasFlag_ = FRAGMENT_DATA_MESSAGE_BODY;
				}
				else
				{
					memcpy(&masks_[data_xy_.second - 1 - web_pFragmentDatasRemain_], pPacket->data() + pPacket->rpos(), web_pFragmentDatasRemain_ - pPacket->totalSize());
					web_pFragmentDatasRemain_ -= pPacket->totalSize();
					pPacket->opfini();
					return;
				}
			}
		}

		PacketReader::processMessages(pMsgHandlers, pPacket);
	}
	*/
}

//-------------------------------------------------------------------------------------
} 
}
