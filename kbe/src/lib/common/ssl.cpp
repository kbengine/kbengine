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
int KB_SSL::isSSLProtocal(MemoryStream* s)
{
	uint8* recvData = s->data();

	/*	Matching SSL/TLS
		SSLv2	0x80	ANY		0x01	0x03
		SSLv3	0x16	0x03	0x00	ANY
		TLS 1.0	0x16	0x03	0x01	ANY
		TLS 1.1	0x16	0x03	0x02	ANY
		TLS 1.2	0x16	0x03	0x03	ANY
	*/
	if (s->length() >= 27 && recvData[2] == 0x01 && recvData[3] == 0x03
		&& (recvData[4] == 0x00 || recvData[4] == 0x01 || recvData[4] == 0x02 || recvData[4] == 0x03)
		&& (s->length() - recvData[1]) == 2)
	{
		// SSLv2 协议
		return SSL2_VERSION;
	}
	else if (s->length() >= 47 && recvData[0] == 0x16 && recvData[1] == 0x03
		&& (recvData[2] == 0x00 || recvData[2] == 0x01 || recvData[2] == 0x02 || recvData[2] == 0x03))
	{
		// SSLv3 协议
		if (recvData[2] == 0x00)
		{
			return SSL3_VERSION;
		}
		else if (recvData[2] == 0x01)
		{
			return TLS1_VERSION;
		}
		else if (recvData[2] == 0x02)
		{
			return TLS1_1_VERSION;
		}
		else if (recvData[2] == 0x03)
		{
			return TLS1_2_VERSION;
		}
	}

	return -1;
}

//-------------------------------------------------------------------------------------
} 
