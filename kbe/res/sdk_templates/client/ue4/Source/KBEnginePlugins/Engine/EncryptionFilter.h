#pragma once
#include "KBECommon.h"

#if PLATFORM_WINDOWS
#include "WindowsHWrapper.h"
#include "AllowWindowsPlatformTypes.h"
#endif

#define UI UI_ST
THIRD_PARTY_INCLUDES_START
#include "openssl/rand.h"
#include "openssl/blowfish.h"
THIRD_PARTY_INCLUDES_END
#undef UI

#if PLATFORM_WINDOWS
#include "HideWindowsPlatformTypes.h"
#endif

namespace KBEngine
{

class MemoryStream;
class PacketSenderBase;
class MessageReader;

class EncryptionFilter
{
public:

	EncryptionFilter() {}
	virtual ~EncryptionFilter();
	virtual void encrypt(MemoryStream *pMemoryStream) = 0;
	virtual void encrypt(uint8 *buf, MessageLengthEx len) = 0;
	virtual void encrypt(uint8 *buf, MessageLengthEx offset, MessageLengthEx len) = 0;

	virtual void decrypt(MemoryStream *pMemoryStream) = 0;
	virtual void decrypt(uint8 *buf, MessageLengthEx len) = 0;
	virtual void decrypt(uint8 *buf, MessageLengthEx offset, MessageLengthEx len) = 0;

	virtual bool send(PacketSenderBase* pPacketSender, MemoryStream *pPacket) = 0;
	virtual bool recv(MessageReader* pMessageReader, MemoryStream *pPacket) = 0;
};


class BlowfishFilter : public EncryptionFilter
{
public:
	// 每块大小
	static const uint32 BLOCK_SIZE = 64 / 8;
	static const uint32 MIN_PACKET_SIZE = (sizeof(MessageLength) + 1 + BLOCK_SIZE);


	// key的最小和最大大小
	static const int MIN_KEY_SIZE = 32 / 8;
	static const int MAX_KEY_SIZE = 448 / 8;

	// 默认key的大小
	static const int DEFAULT_KEY_SIZE = 128 / 8;

	BlowfishFilter(const FString & key);
	BlowfishFilter(int keySize = DEFAULT_KEY_SIZE);

	virtual ~BlowfishFilter();

	virtual void encrypt(MemoryStream *pMemoryStream);
	virtual void encrypt(uint8 *buf, MessageLengthEx len);
	virtual void encrypt(uint8 *buf, MessageLengthEx offset, MessageLengthEx len);

	virtual void decrypt(MemoryStream *pMemoryStream);
	virtual void decrypt(uint8 *buf, MessageLengthEx len);
	virtual void decrypt(uint8 *buf, MessageLengthEx offset, MessageLengthEx len);

	virtual bool send(PacketSenderBase *pPacketSender, MemoryStream *pPacket);
	virtual bool recv(MessageReader *pMessageReader, MemoryStream *pPacket);


	BF_KEY * pBlowFishKey() { return (BF_KEY*)pBlowFishKey_; }

	TArray<uint8> key() 
	{
		TArray<uint8> keyArray;
		keyArray.SetNum(key_.Len());
		memcpy(keyArray.GetData(), TCHAR_TO_ANSI(*key_), key_.Len());

		return keyArray;
	}

private:
	bool init();

private:
	bool			isGood_;
	MemoryStream*	pPacket_;
	MemoryStream*	pEncryptStream_;
	MessageLength	packetLen_;
	uint8			padSize_;

	FString key_;
	int keySize_;
	void * pBlowFishKey_;
};

}
