#pragma once
#include "KBECommon.h"

#ifndef KBENGINE_NO_CRYPTO
// https://stackoverflow.com/questions/51416259/unreal-engine-4-20-build-error-in-plugin-adaptive-unity-build-disabling-pch-f
#pragma warning(disable:4668)   // x  is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
#include "blowfish.h"
#pragma warning(default:4668)  
#include "modes.h"
#endif


class MemoryStream;
class PacketSenderBase;
class MessageReader;

class EncryptionFilter
{
public:
	virtual ~EncryptionFilter() {}

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

	BlowfishFilter(int keySize=16);
	BlowfishFilter(const TArray<uint8>& key);
	virtual ~BlowfishFilter();

	virtual void encrypt(MemoryStream *pMemoryStream) ;
	virtual void encrypt(uint8 *buf, MessageLengthEx len);
	virtual void encrypt(uint8 *buf, MessageLengthEx offset, MessageLengthEx len);

	virtual void decrypt(MemoryStream *pMemoryStream);
	virtual void decrypt(uint8 *buf, MessageLengthEx len);
	virtual void decrypt(uint8 *buf, MessageLengthEx offset, MessageLengthEx len);

	virtual bool send(PacketSenderBase *pPacketSender, MemoryStream *pPacket);
	virtual bool recv(MessageReader *pMessageReader, MemoryStream *pPacket);

	TArray<uint8>& key() {
		return key_;
	}

private:
	bool init();

private:
	bool			isGood_;
	TArray<uint8>	key_;
	MemoryStream*	pPacket_;
	MemoryStream*	pEncryptStream_;
	MessageLength	packetLen_;
	uint8			padSize_;
#ifndef KBENGINE_NO_CRYPTO
	CryptoPP::ECB_Mode<CryptoPP::Blowfish>::Encryption encripter;
	CryptoPP::ECB_Mode<CryptoPP::Blowfish>::Decryption decripter;
#endif
};
