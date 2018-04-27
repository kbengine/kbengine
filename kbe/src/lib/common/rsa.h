// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBENGINE_RSA_H
#define KBENGINE_RSA_H

#include <string>

namespace KBEngine
{


/**
 *	openssl rsaµÄ·â×°
 */
class KBE_RSA
{
public:
	KBE_RSA(const std::string& pubkeyname, 
		const std::string& prikeyname);

	KBE_RSA();

	virtual ~KBE_RSA();

	bool generateKey(const std::string& pubkeyname, 
		const std::string& prikeyname, int keySize = 1024, int e = 65537);

	std::string encrypt(const std::string& instr);
	int encrypt(const std::string& instr, std::string& outCertifdata);
	int decrypt(const std::string& inCertifdata, std::string& outstr);
	std::string decrypt(const std::string& instr);

	static void hexCertifData(const std::string& inCertifdata);

	bool loadPublic(const std::string& keyname);
	bool loadPrivate(const std::string& keyname);

	virtual bool isGood() const { return rsa_public != NULL && rsa_private != NULL; }

protected:
	void* rsa_public, *rsa_private;
};


}

#endif // KBENGINE_RSA_H
