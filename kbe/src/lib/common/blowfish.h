// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com


#ifndef KBENGINE_BLOWFISH_H
#define KBENGINE_BLOWFISH_H

#include "openssl/blowfish.h"
#include <string>

namespace KBEngine { 


class KBEBlowfish
{
public:
	// 每块大小
	static const int BLOCK_SIZE = 64 / 8;

	// key的最小和最大大小
	static const int MIN_KEY_SIZE = 32 / 8;
	static const int MAX_KEY_SIZE = 448 / 8;

	// 默认key的大小
	static const int DEFAULT_KEY_SIZE = 128 / 8;

	typedef std::string Key;

	virtual ~KBEBlowfish();
	KBEBlowfish(const Key & key);
	KBEBlowfish(int keySize = DEFAULT_KEY_SIZE);

	const Key & key() const { return key_; }
	const char * strBlowFishKey() const;
	bool isGood() const { return isGood_; }

	int encrypt(const unsigned char * src, unsigned char * dest, int length);
	int decrypt(const unsigned char * src, unsigned char * dest, int length);

	BF_KEY * pBlowFishKey() { return (BF_KEY*)pBlowFishKey_; }
protected:
	bool init();

	Key key_;
	int keySize_;
	bool isGood_;

	void * pBlowFishKey_;
};

}

#endif // KBENGINE_BLOWFISH_H
