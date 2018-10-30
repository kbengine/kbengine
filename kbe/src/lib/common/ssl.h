// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#ifndef KBE_SSL_H
#define KBE_SSL_H

#include "common/common.h"

namespace KBEngine
{

class MemoryStream;

class KB_SSL
{
public:
	static bool initialize();
	static void finalise();

	/**
		是否为Https/Wss协议
		返回具体协议版本
	*/
	static int isSSLProtocal(MemoryStream* s);

};

}

#endif // KBE_SSL_H
