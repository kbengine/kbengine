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

#include "helper/profile.hpp"
#include "encryption_filter.hpp"
#include "helper/debug_helper.hpp"
#include "network/tcp_packet.hpp"
#include "network/udp_packet.hpp"
#include "network/channel.hpp"
#include "network/network_interface.hpp"
#include "network/packet_receiver.hpp"

namespace KBEngine { 
namespace Mercury
{

//-------------------------------------------------------------------------------------
BlowfishFilter::BlowfishFilter(const Key & key):
KBEBlowfish(key),
pPacket_(NULL),
packetLen_(0),
padSize_(0)
{
}

//-------------------------------------------------------------------------------------
BlowfishFilter::BlowfishFilter():
KBEBlowfish(),
pPacket_(NULL),
packetLen_(0),
padSize_(0)
{
}

//-------------------------------------------------------------------------------------
BlowfishFilter::~BlowfishFilter()
{
	if(pPacket_)
	{
		if(pPacket_->isTCPPacket())
			TCPPacket::ObjPool().reclaimObject(static_cast<TCPPacket *>(pPacket_));
		else
			UDPPacket::ObjPool().reclaimObject(static_cast<UDPPacket *>(pPacket_));

		pPacket_ = NULL;
	}
}

//-------------------------------------------------------------------------------------
Reason BlowfishFilter::send(NetworkInterface & networkInterface, Channel * pChannel, Packet * pPacket)
{
	if(!pPacket->encrypted())
	{
		AUTO_SCOPED_PROFILE("encryptSend")
		
		if (!isGood_)
		{
			WARNING_MSG(fmt::format("BlowfishFilter::send: "
				"Dropping packet to {} due to invalid filter\n",
				pChannel->addr().c_str()));

			return REASON_GENERAL_NETWORK;
		}

		Packet * pOutPacket = NULL;
		if(pPacket->isTCPPacket())
			pOutPacket = TCPPacket::ObjPool().createObject();
		else
			pOutPacket = UDPPacket::ObjPool().createObject();
		
		PacketLength oldlen = pPacket->opsize();
		pOutPacket->wpos(PACKET_LENGTH_SIZE + 1);
		encrypt(pPacket, pOutPacket);

		PacketLength packetLen = pPacket->opsize() + 1;
		uint8 padSize = pPacket->opsize() - oldlen;
		size_t oldwpos =  pOutPacket->wpos();
		pOutPacket->wpos(0);

		(*pOutPacket) << packetLen;
		(*pOutPacket) << padSize;

		pOutPacket->wpos(oldwpos);
		pPacket->swap(*(static_cast<KBEngine::MemoryStream*>(pOutPacket)));

		if(pPacket->isTCPPacket())
			TCPPacket::ObjPool().reclaimObject(static_cast<TCPPacket *>(pOutPacket));
		else
			UDPPacket::ObjPool().reclaimObject(static_cast<UDPPacket *>(pOutPacket));

		/*
		if(Mercury::g_trace_packet > 0)
		{
			DEBUG_MSG(fmt::format("BlowfishFilter::send: packetLen={}, padSize={}\n",
				packetLen, (int)padSize));
		}
		*/
	}

	return networkInterface.basicSendWithRetries(pChannel, pPacket);
}

//-------------------------------------------------------------------------------------
Reason BlowfishFilter::recv(Channel * pChannel, PacketReceiver & receiver, Packet * pPacket)
{
	while(pPacket || pPacket_)
	{
		AUTO_SCOPED_PROFILE("encryptRecv")

		if (!isGood_)
		{
			WARNING_MSG(fmt::format("BlowfishFilter::recv: "
				"Dropping packet to {} due to invalid filter\n",
				pChannel->addr().c_str()));

			return REASON_GENERAL_NETWORK;
		}

		if(pPacket_)
		{
			if(pPacket)
			{
				pPacket_->append(pPacket->data() + pPacket->rpos(), pPacket->opsize());
				
				if(pPacket->isTCPPacket())
					TCPPacket::ObjPool().reclaimObject(static_cast<TCPPacket *>(pPacket));
				else
					UDPPacket::ObjPool().reclaimObject(static_cast<UDPPacket *>(pPacket));
			}

			pPacket = pPacket_;
		}

		if(packetLen_ <= 0)
		{
			// 如果满足一个最小包则尝试解包, 否则缓存这个包待与下一个包合并然后解包
			if(pPacket->opsize() >= (PACKET_LENGTH_SIZE + 1 + BLOCK_SIZE))
			{
				(*pPacket) >> packetLen_;
				(*pPacket) >> padSize_;
				
				packetLen_ -= 1;

				// 如果包是完整下面流出会解密， 如果有多余的内容需要将其剪裁出来待与下一个包合并
				if(pPacket->opsize() > packetLen_)
				{
					if(pPacket->isTCPPacket())
						pPacket_ = TCPPacket::ObjPool().createObject();
					else
						pPacket_ = UDPPacket::ObjPool().createObject();

					pPacket_->append(pPacket->data() + pPacket->rpos() + packetLen_, pPacket->wpos() - (packetLen_ + pPacket->rpos()));
					pPacket->wpos(pPacket->rpos() + packetLen_);
				}
				else if(pPacket->opsize() == packetLen_)
				{
					if(pPacket_ != NULL)
						pPacket_ = NULL;
				}
				else
				{
					if(pPacket_ == NULL)
						pPacket_ = pPacket;

					return receiver.processFilteredPacket(pChannel, NULL);
				}
			}
			else
			{
				if(pPacket_ == NULL)
					pPacket_ = pPacket;

				return receiver.processFilteredPacket(pChannel, NULL);
			}
		}
		else
		{
			// 如果上一次有做过解包行为但包还没有完整则继续处理
			if(pPacket->opsize() > packetLen_)
			{
				if(pPacket->isTCPPacket())
					pPacket_ = TCPPacket::ObjPool().createObject();
				else
					pPacket_ = UDPPacket::ObjPool().createObject();

				pPacket_->append(pPacket->data() + pPacket->rpos() + packetLen_, pPacket->wpos() - (packetLen_ + pPacket->rpos()));
				pPacket->wpos(pPacket->rpos() + packetLen_);
			}
			else if(pPacket->opsize() == packetLen_)
			{
				if(pPacket_ != NULL)
					pPacket_ = NULL;
			}
			else
			{
				if(pPacket_ == NULL)
					pPacket_ = pPacket;

				return receiver.processFilteredPacket(pChannel, NULL);
			}
		}

		decrypt(pPacket, pPacket);

		pPacket->wpos(pPacket->wpos() - padSize_);

		/*
		if(Mercury::g_trace_packet > 0)
		{
			DEBUG_MSG(fmt::format("BlowfishFilter::recv: packetLen={}, padSize={}\n",
				(packetLen_ + 1), (int)padSize_));
		}
		*/

		packetLen_ = 0;
		padSize_ = 0;

		Reason ret = receiver.processFilteredPacket(pChannel, pPacket);
		if(ret != REASON_SUCCESS)
		{
			if(pPacket_)
			{
				if(pPacket_->isTCPPacket())
					TCPPacket::ObjPool().reclaimObject(static_cast<TCPPacket *>(pPacket));
				else
					UDPPacket::ObjPool().reclaimObject(static_cast<UDPPacket *>(pPacket));

				pPacket_ = NULL;
			}
			return ret;
		}

		pPacket = NULL;
	}

	return REASON_SUCCESS;
}

//-------------------------------------------------------------------------------------
void BlowfishFilter::encrypt(Packet * pInPacket, Packet * pOutPacket)
{
	// BlowFish 每次只能加密和解密8字节数据
	// 不足8字节则填充0
	uint8 padSize = 0;

	if (pInPacket->totalSize() % BLOCK_SIZE != 0)
	{
		// 得到不足大小
		padSize = BLOCK_SIZE - (pInPacket->totalSize() % BLOCK_SIZE);

		// 向pPacket中填充这么多
		pInPacket->reserve(pInPacket->size() + padSize);

		// 填充0
		memset(pInPacket->data() + pInPacket->wpos(), 0, padSize);

		pInPacket->wpos(pInPacket->wpos() + padSize);
	}
	
	if(pInPacket != pOutPacket)
	{
		pOutPacket->reserve(pInPacket->size() + pOutPacket->wpos());

		int size = KBEBlowfish::encrypt(pInPacket->data(), pOutPacket->data() + pOutPacket->wpos(),  pInPacket->wpos());
		pOutPacket->wpos(size + pOutPacket->wpos());
	}
	else
	{
		if(pInPacket->isTCPPacket())
			pOutPacket = TCPPacket::ObjPool().createObject();
		else
			pOutPacket = UDPPacket::ObjPool().createObject();

		pOutPacket->reserve(pInPacket->size() + 1);

		int size = KBEBlowfish::encrypt(pInPacket->data(), pOutPacket->data() + pOutPacket->wpos(),  pInPacket->wpos());
		pOutPacket->wpos(size);

		pInPacket->swap(*(static_cast<KBEngine::MemoryStream*>(pOutPacket)));

		if(pInPacket->isTCPPacket())
			TCPPacket::ObjPool().reclaimObject(static_cast<TCPPacket *>(pOutPacket));
		else
			UDPPacket::ObjPool().reclaimObject(static_cast<UDPPacket *>(pOutPacket));
	}

	pInPacket->encrypted(true);
}

//-------------------------------------------------------------------------------------
void BlowfishFilter::decrypt(Packet * pInPacket, Packet * pOutPacket)
{
	if(pInPacket != pOutPacket)
	{
		pOutPacket->reserve(pInPacket->size());

		int size = KBEBlowfish::decrypt(pInPacket->data() + pInPacket->rpos(), 
			pOutPacket->data() + pOutPacket->rpos(),  
			pInPacket->wpos() - pInPacket->rpos());

		pOutPacket->wpos(size + pOutPacket->wpos());
	}
	else
	{
		KBEBlowfish::decrypt(pInPacket->data() + pInPacket->rpos(), pInPacket->data(),  
			pInPacket->wpos() - pInPacket->rpos());

		pInPacket->wpos(pInPacket->wpos() - pInPacket->rpos());
		pInPacket->rpos(0);
	}
}

//-------------------------------------------------------------------------------------

} 
}
