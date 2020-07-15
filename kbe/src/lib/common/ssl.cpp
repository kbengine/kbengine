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
#if (OPENSSL_VERSION_NUMBER >= 0x10002000L) \
    && (OPENSSL_VERSION_NUMBER < 0x10100000L) \
    && !defined(SSL_OP_NO_COMPRESSION)
    ::SSL_COMP_free_compression_methods();
#endif // (OPENSSL_VERSION_NUMBER >= 0x10002000L)

// after 1.1.0 no need
#if (OPENSSL_VERSION_NUMBER <  0x10100000)
// <= 1.0.1f = old api, 1.0.1g+ = new api
#if (OPENSSL_VERSION_NUMBER <= 0x1000106f) || defined(USE_WOLFSSL)
        ERR_remove_state(0);
#else
#if OPENSSL_VERSION_NUMBER >= 0x1010005f && \
    !defined(LIBRESSL_VERSION_NUMBER) && \
    !defined(OPENSSL_IS_BORINGSSL)
        ERR_remove_thread_state();
#else
        ERR_remove_thread_state(NULL);
#endif
#endif
        ERR_free_strings();
        EVP_cleanup();
        CRYPTO_cleanup_all_ex_data();
#endif
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
