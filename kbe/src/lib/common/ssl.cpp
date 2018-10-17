// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "ssl.h"
#include "common/memorystream.h"
#include "helper/debug_helper.h"

//#include <openssl/applink.c>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

namespace KBEngine
{

//-------------------------------------------------------------------------------------
bool KB_SSL::initialize()
{
	SSL_load_error_strings();
	ERR_load_BIO_strings();
	SSL_library_init();
	OpenSSL_add_all_algorithms();
	return true;
}

//-------------------------------------------------------------------------------------
void KB_SSL::finalise()
{
	SSL_COMP_free_compression_methods();

	ERR_remove_state(0);
	ERR_free_strings();
	EVP_cleanup();
	CRYPTO_cleanup_all_ex_data();
}

//-------------------------------------------------------------------------------------
bool KB_SSL::isSSLProtocal(MemoryStream* s)
{
	uint8* recvData = s->data();
	bool isSSL = false;

	if (s->length() >= 27 && recvData[2] == 0x01 && recvData[3] == 0x03
		&& (recvData[4] == 0x00 || recvData[4] == 0x01 || recvData[4] == 0x02 || recvData[4] == 0x03)
		&& (s->length() - recvData[1]) == 2)
	{
		// SSLv2 协议
		isSSL = true;
	}
	else if (s->length() >= 47 && recvData[0] == 0x16 && recvData[1] == 0x03
		&& (recvData[2] == 0x00 || recvData[2] == 0x01 || recvData[2] == 0x02 || recvData[2] == 0x03))
	{
		// SSLv3 协议
		isSSL = true;
	}

	return isSSL;
}

//-------------------------------------------------------------------------------------
} 
