#include "EncryptionFilter.h"
#include "MemoryStream.h"
#include "MessageReader.h"
#include "PacketSenderTCP.h"
#include "PacketSenderKCP.h"
#include "Engine/KBDebug.h"

#include "cryptlib.h"
#include "rdrand.h"
#include "modes.h"
#include "secblock.h"

BlowfishFilter::BlowfishFilter(int keySize):
	isGood_(false),
	pPacket_(new MemoryStream()),
	pEncryptStream_(new MemoryStream()),
	packetLen_(0),
	padSize_(0)
{
	key_.Init(0, keySize);

	CryptoPP::RDRAND rng;
	rng.GenerateBlock(key_.GetData(), key_.Num());
	init();
}

BlowfishFilter::BlowfishFilter(const TArray<uint8>& key):
	isGood_(false),
	key_(key),
	pPacket_(new MemoryStream()),
	pEncryptStream_(new MemoryStream()),
	packetLen_(0),
	padSize_(0)
{
	init();
}

BlowfishFilter::~BlowfishFilter()
{
	KBE_SAFE_RELEASE(pPacket_);
	KBE_SAFE_RELEASE(pEncryptStream_);
}

bool BlowfishFilter::init()
{
	if (key_.Num() >= encripter.MinKeyLength() && key_.Num() <= encripter.MaxKeyLength())
	{
		encripter.SetKey(key_.GetData(), key_.Num());
		decripter.SetKey(key_.GetData(), key_.Num());
		isGood_ = true;
	}
	else
	{
		ERROR_MSG("BlowfishFilter::init: invalid length %d", key_.Num());
		isGood_ = false;
	}

	return isGood_;
}

void BlowfishFilter::encrypt(MemoryStream *pMemoryStream)
{
	// BlowFish 每次只能加密和解密8字节数据
	// 不足8字节则填充0
	uint8 padSize = 0;

	if (pMemoryStream->length() % BLOCK_SIZE != 0)
	{
		padSize = BLOCK_SIZE - pMemoryStream->length() % BLOCK_SIZE;
		pMemoryStream->data_resize(pMemoryStream->size() + padSize);
		memset(pMemoryStream->data() + pMemoryStream->wpos(), 0, padSize);
		pMemoryStream->wpos(pMemoryStream->wpos() + padSize);
	}

	encrypt(pMemoryStream->data() + pMemoryStream->rpos(), pMemoryStream->length());

	uint16 packetLen = pMemoryStream->length() + 1;
	pEncryptStream_->writeUint16(packetLen);
	pEncryptStream_->writeUint8(padSize);
	pEncryptStream_->append(pMemoryStream->data() + pMemoryStream->rpos(), pMemoryStream->length());

	pMemoryStream->swap(*pEncryptStream_);
	pEncryptStream_->clear(false);
}

void BlowfishFilter::encrypt(uint8 *buf, MessageLengthEx len)
{
	if (len % BLOCK_SIZE != 0)
	{
		ERROR_MSG("BlowfishFilter::encrypt: Input length (%d) is not a multiple of block size ", len);
		return;
	}

	uint8* data = buf;
	uint64 prevBlock = 0;
	for (uint32 i = 0; i < len; i += BLOCK_SIZE)
	{
		if (prevBlock != 0)
		{
			uint64 oldValue = *(uint64*)(data + i);
			*(uint64*)(data + i) = *(uint64*)(data + i) ^ (prevBlock);
			prevBlock = oldValue;
		}
		else
		{
			prevBlock = *(uint64*)(data + i);
		}

		encripter.ProcessData(data + i, data + i, BLOCK_SIZE);
	}
}

void BlowfishFilter::encrypt(uint8 *buf, MessageLengthEx offset, MessageLengthEx len)
{
	encrypt(buf + offset, len);
}

void BlowfishFilter::decrypt(MemoryStream *pMemoryStream)
{
	decrypt(pMemoryStream->data() + pMemoryStream->rpos(), pMemoryStream->length());
}

void BlowfishFilter::decrypt(uint8 *buf, MessageLengthEx len)
{
	if (len % BLOCK_SIZE != 0)
	{
		ERROR_MSG("BlowfishFilter::decrypt: Input length (%d) is not a multiple of block size ", len);
		return;
	}

	uint8* data = buf;
	uint64 prevBlock = 0;
	for (uint32 i = 0; i < len; i += BLOCK_SIZE)
	{
		decripter.ProcessData(data + i, data + i, BLOCK_SIZE);

		if (prevBlock != 0)
		{
			*(uint64*)(data + i) = *(uint64*)(data + i) ^ (prevBlock);
		}
		
		prevBlock = *(uint64*)(data + i);
	}
}

void BlowfishFilter::decrypt(uint8 *buf, MessageLengthEx offset, MessageLengthEx len)
{
	decrypt(buf + offset, len);
}

bool BlowfishFilter::send(PacketSenderBase* pPacketSender, MemoryStream *pPacket)
{
	if (!isGood_)
	{
		ERROR_MSG("BlowfishFilter::send: Dropping packet due to invalid filter");
		return false;
	}

	encrypt(pPacket);

	return pPacketSender->send(pPacket);;
}

bool BlowfishFilter::recv(MessageReader* pMessageReader, MemoryStream *pPacket)
{
	if (!isGood_)
	{
		ERROR_MSG("BlowfishFilter::recv: Dropping packet due to invalid filter");
		return false;
	}

	uint32 oldrpos = pPacket->rpos();
	uint32 len = pPacket->length();
	uint16 packeLen = pPacket->readUint16();

	if ( 0 == pPacket_->length() && len > MIN_PACKET_SIZE && packeLen - 1 == len - 3)
	{
		int padSize = pPacket->readUint8();
		decrypt(pPacket);

		if (pMessageReader)
		{
			pMessageReader->process(pPacket->data() + pPacket->rpos(), 0, pPacket->length() - padSize);
		}

		pPacket->clear(false);
		return true;
	}

	pPacket->rpos(oldrpos);
	pPacket_->append(pPacket->data() + pPacket->rpos(), pPacket->length());
	pPacket->clear(false);

	while (pPacket_->length() > 0)
	{
		uint32 currLen = 0;
		int oldwpos = 0;
		if (packetLen_ <= 0)
		{
			if (pPacket_->length() >= MIN_PACKET_SIZE)
			{
				(*pPacket_) >> packetLen_;
				(*pPacket_) >> padSize_;

				packetLen_ -= 1;

				if (pPacket_->length() > packetLen_)
				{
					currLen = (uint32)(pPacket_->rpos() + packetLen_);
					oldwpos = pPacket_->wpos();
					pPacket_->wpos(currLen);
				}
				else if (pPacket_->length() < packetLen_)
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}
		else
		{
			if (pPacket_->length() > packetLen_)
			{
				currLen = (uint32)(pPacket_->rpos() + packetLen_);
				oldwpos = pPacket_->wpos();
				pPacket_->wpos(currLen);
			}
			else if (pPacket_->length() < packetLen_)
			{
				return false;
			}
		}

		decrypt(pPacket_);
		pPacket_->wpos(pPacket_->wpos() - padSize_);

		if (pMessageReader)
		{
			pMessageReader->process(pPacket_->data() + pPacket_->rpos(), 0, pPacket_->length());
		}

		if (currLen > 0)
		{
			pPacket_->rpos(currLen);
			pPacket_->wpos(oldwpos);
		}
		else
		{
			pPacket_->clear(false);
		}

		packetLen_ = 0;
		padSize_ = 0;
	}
	
	return true;
}

